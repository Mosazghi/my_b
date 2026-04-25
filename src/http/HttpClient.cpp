#include "HttpClient.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <cassert>
#include <http/HttpClient.hpp>
// #include <zlib.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <format>
#include <optional>
#include <regex>
#include <utility>
#include "Types.hpp"
#include "logger.hpp"
#include "utils.hpp"

namespace http {

HttpClient::HttpClient() = default;

HttpClient::~HttpClient() {
  for (const auto& [_, v] : m_http_sockets) {
    close(v.first);
    freeaddrinfo(v.second);
  }

  for (const auto& [k, v] : m_https_sockets) {
    BIO_free_all(v.first);
    SSL_CTX_free(v.second);
  }
}

HttpResult HttpClient::get(std::string_view url) {
  auto params = get_params_from_url(url);
  auto cache_key = get_cache_key(params.value());  // Add null check?
  HttpResult result{};

  if (!params.has_value()) {
    logger.err("Failed to get HTTP request parameters from URL");
    result.errors.emplace_back(
        "Failed to get HTTP request parameters from URL");
    return result;
  }

  if (m_response_cache.contains(cache_key)) {
    auto cache = m_response_cache.at(cache_key);
    auto now = std::chrono::system_clock::now();
    bool expired = now > cache.timestamp + std::chrono::seconds(cache.max_age);
    if (expired) {
      m_response_cache.erase(cache_key);
    } else {
      return HttpResult{
          .response =
              HttpResponse{.status_line = HttpStatusLine{.version = "HTTP/1.1",
                                                         .explanation = "OK",
                                                         .status = 200},
                           .headers = cache.headers,
                           .body = cache.body},
          .errors = {},
          .redirect_count = 0,
      };
    }
  }

  auto perform_request = [&](const HttpReqParams& params) -> HttpResult {
    static int num_requests = 0;
    num_requests++;
    std::optional<http::HttpResponse> response{};

    HttpRequestBuilder builder;

    const auto buffer = builder.with_method(HttpMethod::GET)
                            .with_path(params.path)
                            .with_header("Host", params.hostname)
                            .with_header("User-Agent", "mosa")
                            .with_header("Accept-Encoding", "gzip")
                            .with_header("Connection", "keep-alive")
                            .build();

    logger.dbg("Sending:\n{}", buffer);

    switch (params.scheme) {
      case url::Scheme::HTTP:
        response = http_req(params, buffer);
        break;
      case url::Scheme::HTTPS:
        response = https_req(params, buffer);
        break;
      default:
        logger.err("Unknown scheme");
        break;
    }

    HttpResult result{};

    if (!response.has_value()) {
      result.errors.emplace_back("Request failed");
      return result;
    }

    result.response = response.value();

    const std::regex content_encoding_regex(
        R"(\s*([a-zA-Z0-9_-]+)\s*(?:,\s*([a-zA-Z0-9_-]+)\s*)*)",
        std::regex::ECMAScript | std::regex::icase);
    std::smatch m;

    if (result.response.headers.contains("content-encoding") &&
        std::regex_search(result.response.headers.at("content-encoding"), m,
                          content_encoding_regex)) {
      std::string text_output;
      auto decompressed = utils::ungzip(result.response.body);
      if (!decompressed.has_value()) {
        logger.err("Decompressing falied");
        result.errors.emplace_back("Decompressing failed");
      }

      result.response.body = decompressed.value_or("");
      result.redirect_count = num_requests - 1;
    }

    return result;
  };

  result = perform_request(params.value());

  if (result.has_error()) {
    logger.err("Some error occurred");
    return result;
  }

  int redirects_num = 0;
  auto last_params = params.value();
  while (should_redirect(result.response) &&
         std::cmp_less(redirects_num, MAX_CONSECUTIVE_REDIRECTS)) {
    logger.dbg("Redirecting to {}", result.response.headers.at("location"));

    if (!result.response.headers.contains("location")) {
      break;
    }

    std::string loc;
    loc = result.response.headers.at("location");
    // m_last_redirect = true;
    if (loc.at(0) == '/') {
      last_params = params.value();
      last_params.path = loc;
    } else {
      last_params = get_params_from_url(loc).value();
    }

    result = perform_request(last_params);
    // result.redirect_count++;
  }

  const bool should_cache = !m_response_cache.contains(cache_key) &&
                            result.response.status_line.status == 200;

  if (should_cache) {
    std::regex re(R"((?:^|[\s,])max-age\s*=\s*(\d+))", std::regex::icase);
    std::smatch m;
    uint32_t max_age = 0;
    if (result.response.headers.contains("cache-control")) {
      auto cache_ctrl_str = result.response.headers.at("cache-control");
      if (std::regex_search(cache_ctrl_str, m, re)) {
        max_age = std::stoi(m[1].str());
      }
      m_response_cache[cache_key] =
          HttpRespCache{.headers = result.response.headers,
                        .timestamp = std::chrono::system_clock::now(),
                        .body = result.response.body,
                        .max_age = max_age};
    }
  }

  return result;
}

uint16_t HttpClient::get_status_code(const std::string& header) const {
  std::regex sl_regex(R"(HTTP\/\S+\s+(\d{3}))");
  std::smatch m;

  if (std::regex_search(header, m, sl_regex)) {
    try {
      return std::stoi(m[1].str());
    } catch (const std::exception& e) {
      logger.err("Error converting status code {}", e.what());
      return 500;
    }
  }

  return 404;
}

std::optional<http::HttpReqParams> HttpClient::get_params_from_url(
    std::string_view url) const {
  http::HttpReqParams params{};
  auto s1 = url.find("://");
  std::string_view scheme = url.substr(0, s1);

  std::string_view rest = url.substr(s1 + 3);

  auto s2 = rest.find('/');

  if (scheme == "http") {
    params.scheme = url::Scheme::HTTP;
  } else if (scheme == "https") {
    params.scheme = url::Scheme::HTTPS;
  } else {
    logger.warn("Unknwon scheme");
    return {};
  }

  if (s2 == std::string::npos) {
    params.path = "/";
    params.hostname = url.substr(s1 + 3);
  } else {
    params.hostname = rest.substr(0, s2);
    params.path = rest.substr(s2);
  }

  // Custom port
  auto c_port = params.hostname.find_first_of(':');
  if (c_port != std::string::npos) {
    params.port = std::stoi(params.hostname.substr(c_port + 1));
    params.hostname = params.hostname.substr(0, c_port);
  } else {
    params.port = params.scheme == url::Scheme::HTTP ? 80 : 443;
  }

  logger.dbg("Host: {}", params.hostname);
  logger.dbg("Port: {}", params.port);
  logger.dbg("Path: {}", params.path);
  logger.dbg("Scheme: {}",
             params.scheme == url::Scheme::HTTP ? "HTTP" : "HTTPS");
  return params;
}

std::optional<HttpResponse> HttpClient::http_req(const HttpReqParams& params,
                                                 std::string_view buffer) {
  std::string key = get_cache_key(params);
  bool cache_hit = m_https_sockets.contains(key);
  int sockfd;

  if (cache_hit) {
    sockfd = m_http_sockets.at(key).first;
  } else {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
  }

  if (sockfd < 0) {
    logger.err("Connection failed!");
    return {};
  }

  addrinfo hints{};
  hints.ai_family = AF_INET;  // IPv4
  hints.ai_socktype = SOCK_STREAM;

  addrinfo* res;
  if (!cache_hit) {
    if (getaddrinfo(params.hostname.c_str(),
                    std::to_string(params.port).c_str(), &hints, &res) != 0) {
      logger.warn("DNS lookup failed");
      return {};
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
      logger.warn("Connection failed");
      return {};
    }
  }

  send(sockfd, buffer.data(), buffer.size(), 0);

  auto [header, body] = get_header_body(read, sockfd);
  std::string response = header + body;
  int code = get_status_code(header);

  if (!cache_hit) {
    m_http_sockets[key] = std::make_pair(sockfd, res);
  }

  // Generate headers
  std::regex header_re(R"(([A-Za-z0-9-]+):\s*(.*?)\s*\r?\n)");
  std::smatch m;
  std::unordered_map<std::string, std::string> headers;

  std::sregex_iterator begin(header.begin(), header.end(), header_re);
  std::sregex_iterator end;

  for (std::sregex_iterator i = begin; i != end; ++i) {
    const std::smatch& match = *i;

    std::string key = match.str(1);
    std::ranges::transform(key, key.begin(), ::tolower);

    std::string value = match.str(2);

    headers[key] = value;
  }
  return HttpResponse{
      .status_line =
          HttpStatusLine{.version = "HTTP/1.1",
                         .explanation = "OK",  // TODO parse explanation
                         .status = code},
      .headers = headers,
      .body = body};
}

std::optional<HttpResponse> HttpClient::https_req(HttpReqParams params,
                                                  std::string_view buffer) {
  std::string key = get_cache_key(params);

  BIO* bio;
  SSL_CTX* ctx;

  auto it = m_https_sockets.find(key);
  bool cache_hit = it != m_https_sockets.end();

  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();

  if (cache_hit) {
    ctx = m_https_sockets.at(key).second;
  } else {
    ctx = SSL_CTX_new(SSLv23_client_method());
  }

  if (ctx == nullptr) {
    logger.warn("SSL CTX is null!");
    return {};
  }

  if (cache_hit) {
    bio = m_https_sockets.at(key).first;
  } else {
    bio = BIO_new_ssl_connect(ctx);
    BIO_set_conn_hostname(
        bio, std::format("{}:{}", params.hostname, std::to_string(params.port))
                 .c_str());

    if (BIO_do_connect(bio) <= 0) {
      logger.warn("Failed connection");
      return std::nullopt;
    }
  }

  if (BIO_write(bio, buffer.data(), buffer.size()) <= 0) {
    if (!BIO_should_retry(bio)) {
      // Not worth implementing, but worth knowing.
    }

    logger.warn("Failed write");
  }

  auto [header, body] = get_header_body(BIO_read, bio);
  std::string response = header + body;
  int code = get_status_code(header);

  if (!cache_hit) {
    m_https_sockets[key] = std::make_pair(bio, ctx);
  }

  // Generate headers
  std::regex header_re(R"(([A-Za-z0-9-]+):\s*(.*?)\s*\r?\n)");
  std::smatch m;
  std::unordered_map<std::string, std::string> headers;

  std::sregex_iterator begin(header.begin(), header.end(), header_re);
  std::sregex_iterator end;

  for (std::sregex_iterator i = begin; i != end; ++i) {
    const std::smatch& match = *i;

    std::string key = match.str(1);
    std::ranges::transform(key, key.begin(), ::tolower);

    std::string value = match.str(2);

    headers[key] = value;
  }

  return HttpResponse{
      .status_line =
          HttpStatusLine{.version = "HTTP/1.1",
                         .explanation = "OK",  // TODO parse explanation
                         .status = code},
      .headers = headers,
      .body = body};
}

template <typename ReadFunc, typename Stream>
std::pair<std::string, std::string> HttpClient::get_header_body(
    ReadFunc func, Stream stream) const {
  std::string header_buffer{};
  char c;
  int bytes_read = 0;

  while ((bytes_read = func(stream, &c, 1)) > 0) {
    header_buffer.push_back(c);
    if (header_buffer.size() >= 4 &&
        header_buffer.substr(header_buffer.size() - 4) == "\r\n\r\n") {
      break;
    }
  }
  if (bytes_read <= 0 && header_buffer.size() < 4) {
    logger.warn("connection closed during header read.");
    return {};
  }

  size_t content_length{get_content_len(header_buffer)};

  bool is_chunked{false};

  std::regex te_regex(R"(Transfer-Encoding:\s*chunked)", std::regex::icase);
  if (std::regex_search(header_buffer, te_regex)) {
    is_chunked = true;
    logger.dbg("Is chnked");
  }

  std::string body_buffer{};
  if (is_chunked) {
    // Read a single line from the chunked stream
    auto read_line = [&]() {
      std::string line;
      char c;
      while (func(stream, &c, 1) > 0) {
        line.push_back(c);
        if (line.size() >= 2 && line.substr(line.size() - 2) == "\r\n") {
          line.pop_back();
          line.pop_back();
          break;
        }
      }
      return line;
    };

    while (true) {
      // 1. Read the chunk size
      std::string size_line = read_line();
      if (size_line.empty()) {
        break;
      }
      size_t chunk_size{};
      try {
        chunk_size = std::stoul(size_line, nullptr, 16);
      } catch (...) {
        break;
      }

      if (chunk_size == 0) {
        read_line();
        break;
      }

      // 2. Read up to 0..chunk_size
      size_t total_read{};
      std::string chunk_data(chunk_size, '\0');
      while (total_read < chunk_size) {
        int size = func(stream, chunk_data.data() + total_read,
                        chunk_size - total_read);
        if (size <= 0) {
          break;
        }
        total_read += size;
      }

      body_buffer.append(chunk_data);

      // 3. Read trailing "\r\n"
      read_line();
    }
  } else {
    body_buffer.resize(content_length);
    size_t total_bytes_read = 0;
    while (total_bytes_read < content_length) {
      int size = func(stream, body_buffer.data() + total_bytes_read,
                      content_length - total_bytes_read);
      if (size <= 0) {
        break;
      }
      total_bytes_read += size;
    }
  }

  return {header_buffer, body_buffer};
}

uint16_t HttpClient::get_content_len(const std::string& header) const {
  uint16_t content_length{};
  std::regex cl_regex(R"([Cc]ontent-[Ll]ength:\s*(\d+)\r?\n)");
  std::smatch m;

  if (std::regex_search(header, m, cl_regex) && m.size() > 1) {
    try {
      content_length = std::stol(m[1].str());
    } catch (const std::exception& e) {
      logger.err("Failed to read Content Legnth.");
      return 0;
    }
  }
  return content_length;
}

}  // namespace http
