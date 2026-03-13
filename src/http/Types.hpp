#pragma once
#include <chrono>
#include <string>
#include <unordered_map>
#include <url/Types.hpp>

namespace http {
using Headers = std::unordered_map<std::string, std::string>;

enum class HttpMethod { GET, POST, PUT, DELETE };

inline const std::string http_method_to_string(HttpMethod method) {
  switch (method) {
    case HttpMethod::GET:
      return "GET";
    case HttpMethod::POST:
      return "POST";
    case HttpMethod::PUT:
      return "PUT";
    case HttpMethod::DELETE:
      return "DELETE";
    default:
      return "UNKNOWN";
  }
}

struct HttpRequestBuilder {
  HttpMethod method;
  std::string path;
  Headers headers;
  std::string body;

  HttpRequestBuilder& with_method(HttpMethod m) {
    method = m;
    return *this;
  }
  HttpRequestBuilder& with_path(const std::string& p) {
    path = p;
    return *this;
  }
  HttpRequestBuilder& with_header(const std::string& key,
                                  const std::string& value) {
    headers[key] = value;
    return *this;
  }
  HttpRequestBuilder& with_body(const std::string& b) {
    body = b;
    return *this;
  }

  std::string build() const {
    std::string request =
        http_method_to_string(method) + " " + path + " HTTP/1.1\r\n";
    for (const auto& [key, value] : headers) {
      request += key + ": " + value + "\r\n";
    }
    request += "\r\n";
    request += body;
    return request;
  }
};

enum class HttpPort {
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

  bool is_success() const {
    return !has_error() && response.status_line.status >= 200 &&
           response.status_line.status < 300;
  }

  bool has_error() const {
    return !errors.empty() || response.status_line.status >= 400;
  }
};

struct HttpRespCache {
  Headers headers;
  std::chrono::system_clock::time_point timestamp;
  std::string body;
  uint32_t max_age;
};

}  // namespace http
