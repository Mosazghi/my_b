#pragma once
#include <memory>
#include <string>
// #include "http_client.h"
#include "file.h"
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
  FILE,
};

class URL {
 public:
  URL(const std::string& url, std::shared_ptr<http::HttpClient> http_client,
      std::shared_ptr<file::File> file_client);
  ~URL();
  std::optional<http::HttpResponse> request();
  void show(const std::string& body);

 private:
  struct Data {
    Scheme scheme;
    std::string host;
    std::optional<int> port;
    std::string path;
  } m_data;

  Logger* logger;
  std::shared_ptr<http::HttpClient> m_http_client;
  std::shared_ptr<file::File> m_file_client;
};
}  // namespace url
