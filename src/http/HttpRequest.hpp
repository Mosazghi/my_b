#pragma once
#include <chrono>
#include <string>
#include <unordered_map>
#include "url/Types.hpp"

namespace http {
using Headers = std::unordered_map<std::string, std::string>;

enum class HttpMethod : std::uint8_t { GET, POST, PUT, DELETE };

std::string http_method_to_string(HttpMethod method);

struct HttpRequestBuilder {
  HttpMethod method;
  std::string path;
  Headers headers;
  std::string body;

  HttpRequestBuilder& with_method(HttpMethod m);
  HttpRequestBuilder& with_path(const std::string& p);
  HttpRequestBuilder& with_header(const std::string& key,
                                  const std::string& value);
  HttpRequestBuilder& with_body(const std::string& b);
  std::string build() const;
};

enum class HttpPort : std::uint16_t {
  HTTP = 80,
  HTTPS = 443,
};

struct HttpReqParams {
  int port;
  std::string hostname;
  std::string path;
  url::Scheme scheme;
};

struct HttpStatusLine {
  std::string version;      // e.g., "HTTP/1.1"
  std::string explanation;  // e.g., "OK", "Not Found"
  int status;               // e.g., 200, 404
};

// Core HTTP response data (what the server sent)
struct HttpResponse {
  HttpStatusLine status_line;
  Headers headers;
  std::string body;
};

// Client-side response wrapper (includes metadata)
struct HttpResult {
  using Errors = std::vector<std::string>;

  HttpResponse response;
  Errors errors;
  int redirect_count;

  bool is_success() const;
  bool has_error() const;
};

struct HttpRespCache {
  Headers headers;
  std::chrono::system_clock::time_point timestamp;
  std::string body;
  uint32_t max_age;
};

}  // namespace http
