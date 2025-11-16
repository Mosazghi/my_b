#pragma once
#include <memory>
#include <string>
// #include "http_client.h"
#include "logger.h"

// Forward declarations
namespace http {
class HttpClient;
struct HttpResponse;
}  // namespace http

namespace url {

enum class Scheme {
  UNKNOWN,  //< Unsupported.
  HTTP,
  HTTPS,
};

class URL {
 public:
  URL(std::string const& url, std::shared_ptr<http::HttpClient> http_client);
  ~URL();
  http::HttpResponse request();
  void show(const std::string& body);

 private:
  Scheme m_scheme{};
  std::string m_hostname{};
  std::string m_path{};
  int m_port;
  Logger* logger;
  std::shared_ptr<http::HttpClient> m_http_client;
};
}  // namespace url
