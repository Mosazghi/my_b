#include "HttpRequest.hpp"
#include <format>

namespace http {

std::string http_method_to_string(HttpMethod method) {
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

HttpRequestBuilder& HttpRequestBuilder::with_method(HttpMethod m) {
  method = m;
  return *this;
}
HttpRequestBuilder& HttpRequestBuilder::with_path(const std::string& p) {
  path = p;
  return *this;
}
HttpRequestBuilder& HttpRequestBuilder::with_header(const std::string& key,
                                                    const std::string& value) {
  headers[key] = value;
  return *this;
}
HttpRequestBuilder& HttpRequestBuilder::with_body(const std::string& b) {
  body = b;
  return *this;
}
std::string HttpRequestBuilder::build() const {
  std::string request =
      http_method_to_string(method) + " " + path + " HTTP/1.1\r\n";
  for (const auto& [key, value] : headers) {
    request.append(std::format("{}: {}\r\n", key, value));
  }
  request += "\r\n";
  request += body;
  return request;
}

bool HttpResult::is_success() const {
  return !has_error() && response.status_line.status >= 200 &&
         response.status_line.status < 300;
}
bool HttpResult::has_error() const {
  return !errors.empty() || response.status_line.status >= 400;
}

}  // namespace http
