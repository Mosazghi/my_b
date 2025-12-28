#include "HttpClient.h"
#include <arpa/inet.h>
#include <assert.h>
#include <http/HttpClient.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <zlib.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <format>
#include <optional>
#include <regex>
#include "Types.h"
#include "logger.h"
#include "utils.h"

namespace http {

HttpClient::HttpClient() { logger = new Logger("HttpClient"); }

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

std::optional<HttpResponse> HttpClient::get(const std::string& url) {
  std::optional<http::HttpResponse> resp{};
  auto params = m_last_redirect ? m_last_params : get_params_from_url(url);
  auto cache_key = get_cache_key(params.value());

  if (!params.has_value()) {
    return {};
  }

  if (!m_last_redirect) {
    if (m_resp_cache.contains(cache_key)) {
      auto cache = m_resp_cache.at(cache_key);
      auto now = std::chrono::system_clock::now();
      bool expired =
          now > cache.timestamp + std::chrono::seconds(cache.max_age);
      if (expired) {
        m_resp_cache.erase(cache_key);
      } else {
        return HttpResponse{
            .code = 200,
            .body = cache.body,
            .headers = cache.headers,
        };
      }
    }
  }

  std::string buffer = std::format("GET {} HTTP/1.1\r\n", params.value().path);
  buffer.append(std::format("Host: {}\r\n", params.value().hostname));
  buffer.append("User-Agent: mosa\r\n");
  buffer.append("Accept-Encoding: gzip\r\n");
  buffer.append("Connection: keep-alive\r\n");
  buffer.append("\r\n");

  logger->dbg("Sending:\n{}", buffer);

  switch (params.value().scheme) {
    case url::Scheme::HTTP:
      resp = http_req(params.value(), buffer);
      break;
    case url::Scheme::HTTPS:
      resp = https_req(params.value(), buffer);
      break;
    default:
      logger->err("Unknown scheme");
      break;
  }

  if (resp.has_value()) {
    // check for compression
    const std::regex content_encoding_regex(
        R"(\s*([a-zA-Z0-9_-]+)\s*(?:,\s*([a-zA-Z0-9_-]+)\s*)*)",
        std::regex::ECMAScript | std::regex::icase);
    std::smatch m;

    if (resp->headers.contains("content-encoding") &&
        std::regex_search(resp->headers.at("content-encoding"), m,
                          content_encoding_regex)) {
      std::string text_output;
      auto res = utils::ungzip(resp->body);
      if (!res.has_value()) {
        logger->err("Decompressing falied");
      }
      resp->body = res.value_or("");
    }

    // TODO: refactor redirect logic
    if (should_redirect(resp.value())) {
      if (m_redirect_counts >= MAX_CONSECUTIVE_REDIRS) {
        logger->warn("Too many redirects. Halting further requests.");
        return {};
      }

      std::string loc;
      if (resp->headers.contains("location")) {
        loc = resp->headers.at("location");
        m_last_redirect = true;
        if (loc.at(0) == '/') {
          m_last_params = params.value();
          m_last_params.path = loc;
        } else {
          m_last_params = get_params_from_url(loc).value();
        }

        m_redirect_counts++;
        get(loc);
      }
    } else {
      m_last_redirect = false;
      m_redirect_counts = 0;
    }
  }

  const bool should_cache =
      !m_resp_cache.contains(cache_key) && resp->code == 200;

  if (should_cache) {
    std::regex re(R"((?:^|[\s,])max-age\s*=\s*(\d+))", std::regex::icase);
    std::smatch m;
    uint32_t max_age = 0;
    if (resp->headers.contains("cache-control")) {
      auto cache_ctrl_str = resp->headers.at("cache-control");
      if (std::regex_search(cache_ctrl_str, m, re)) {
        max_age = std::stoi(m[1].str());
      }
      m_resp_cache[cache_key] = HttpRespCache{
          resp->body, resp->headers, std::chrono::system_clock::now(), max_age};
    }
  }

  return resp;
}

bool HttpClient::should_redirect(const HttpResponse& r) const {
  return (r.code >= 300 && r.code <= 399);
}

uint16_t HttpClient::get_status_code(const std::string& header) const {
  std::regex sl_regex(R"(HTTP\/\S+\s+(\d{3}))");
  std::smatch m;

  if (std::regex_search(header, m, sl_regex)) {
    try {
      return std::stoi(m[1].str());
    } catch (const std::exception& e) {
      logger->err("Error converting status code {}", e.what());
      return 500;
    }
  }

  return 404;
}

std::optional<http::HttpReqParams> HttpClient::get_params_from_url(
    const std::string& url) const {
  http::HttpReqParams params{};
  auto s1 = url.find("://");
  std::string scheme = url.substr(0, s1);

  std::string rest = url.substr(s1 + 3);

  auto s2 = rest.find("/");

  if (scheme == "http") {
    params.scheme = url::Scheme::HTTP;
  } else if (scheme == "https") {
    params.scheme = url::Scheme::HTTPS;
  } else {
    logger->warn("Unknwon scheme");
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
  auto c_port = params.hostname.find_first_of(":");
  if (c_port != std::string::npos) {
    params.port = std::stoi(params.hostname.substr(c_port + 1));
    params.hostname = params.hostname.substr(0, c_port);
  } else {
    params.port = params.scheme == url::Scheme::HTTP ? 80 : 443;
  }

  logger->dbg("Host: {}", params.hostname);
  logger->dbg("Port: {}", params.port);
  logger->dbg("Path: {}", params.path);
  logger->dbg("Scheme: {}",
              params.scheme == url::Scheme::HTTP ? "HTTP" : "HTTPS");
  return params;
}

std::optional<HttpResponse> HttpClient::http_req(const HttpReqParams& params,
                                                 const std::string& buffer) {
  std::string key = get_cache_key(params);
  bool cache_hit = m_https_sockets.contains(key);
  int sockfd;

  if (cache_hit) {
    sockfd = m_http_sockets.at(key).first;
  } else {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
  }

  if (sockfd < 0) {
    logger->err("Connection failed!");
    exit(EXIT_FAILURE);
  }

  addrinfo hints{};
  hints.ai_family = AF_INET;  // IPv4
  hints.ai_socktype = SOCK_STREAM;

  addrinfo* res;
  if (!cache_hit) {
    if (getaddrinfo(params.hostname.c_str(),
                    std::to_string(params.port).c_str(), &hints, &res) != 0) {
      logger->warn("DNS lookup failed");
      return {};
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
      logger->warn("Connection failed");
      return {};
    }
  }

  send(sockfd, buffer.c_str(), buffer.size(), 0);

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
    std::smatch match = *i;

    std::string key = match.str(1);
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    std::string value = match.str(2);

    headers[key] = value;
  }
  return HttpResponse{code, body, headers};
}

std::optional<HttpResponse> HttpClient::https_req(HttpReqParams params,
                                                  const std::string& buffer) {
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

  if (ctx == NULL) {
    logger->warn("SSL CTX is null!");
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
      logger->warn("Failed connection");
      return std::nullopt;
    }
  }

  if (BIO_write(bio, buffer.c_str(), strlen(buffer.c_str())) <= 0) {
    if (!BIO_should_retry(bio)) {
      // Not worth implementing, but worth knowing.
    }

    logger->warn("Failed write");
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
    std::smatch match = *i;

    std::string key = match.str(1);
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    std::string value = match.str(2);

    headers[key] = value;
  }

  return HttpResponse{code, body, headers};
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
    logger->warn("connection closed during header read.");
    return {};
  }

  size_t content_length{get_content_len(header_buffer)};

  bool is_chunked{false};

  std::regex te_regex(R"(Transfer-Encoding:\s*chunked)", std::regex::icase);
  if (std::regex_search(header_buffer, te_regex)) {
    is_chunked = true;
    logger->dbg("Is chnked");
  }

  std::string body_buffer{};
  logger->dbg("Content len: {}", content_length);
  if (is_chunked) {
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

    while (1) {
      // 1. Read the chunk size
      std::string size_line = read_line();
      if (size_line.empty()) break;
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
      logger->err("Failed to read Content Legnth.");
      return 0;
    }
  }
  return content_length;
}

std::string HttpClient::get_cache_key(const HttpReqParams& p) const {
  return std::format("{}:{}", p.hostname, p.port);
}

}  // namespace http
