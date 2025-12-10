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
#include <cstring>
#include <format>
#include <optional>
#include <regex>
#include "Types.h"
#include "logger.h"
#include "url/Url.h"

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
  auto params = get_params_from_url(url);

  if (!params.has_value()) {
    return {};
  }

  std::string buffer = std::format("GET {} HTTP/1.1\r\n", params.value().path);
  buffer.append(std::format("Host: {}\r\n", params.value().hostname));
  buffer.append("User-Agent: mosa\r\n");
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

  return resp;
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

std::optional<HttpResponse> HttpClient::http_req(HttpReqParams params,
                                                 const std::string& buffer) {
  std::string key = get_cache_key(params);
  auto it = m_https_sockets.find(key);
  bool cache_hit = it != m_https_sockets.end();
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

  return HttpResponse{code, response};
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

  return HttpResponse{code, response};
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

  uint16_t content_length{get_content_len(header_buffer)};

  std::string body_buffer{};
  body_buffer.resize(content_length);
  int total_bytes_read = 0;
  while (total_bytes_read < content_length) {
    int size = func(stream, body_buffer.data() + total_bytes_read,
                    content_length - total_bytes_read);
    if (size <= 0) {
      break;
    }
    total_bytes_read += size;
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

std::string HttpClient::get_cache_key(HttpReqParams p) const {
  return std::format("{}:{}", p.hostname, p.port);
}

}  // namespace http
