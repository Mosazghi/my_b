#pragma once
#include <memory>
#include <string>
#include "IUrl.h"
#include "Types.h"
#include "http/IHttpClient.h"
#include "logger.h"

// Forward declarations
namespace http {
class HttpClient;
class IHttpClient;
struct HttpResponse;
}  // namespace http

namespace url {

class URL : public IUrl {
 public:
  URL(const std::string& url, std::shared_ptr<http::IHttpClient> http_client);
  ~URL();
  std::optional<http::HttpResponse> request();
  void show(const std::string& body);

 private:
  struct Data {
    Scheme scheme;
    std::string host;
    std::optional<int> port;
    std::string path;
    // TODO: Refactor
    struct DataScheme {
      std::string protocol;
      std::string data;
    } data_scheme;
  } m_data;

  std::string m_url{};

  Logger* logger;
  std::shared_ptr<http::IHttpClient> m_http_client;

  bool is_scheme_in(Scheme scheme);
  bool is_scheme_in(const std::vector<Scheme>& schemes);
};
}  // namespace url
