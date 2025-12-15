#pragma once
#include <memory>
#include <string>
#include "IUrl.h"
#include "Types.h"
#include "http/IHttpClient.h"
#include "logger.h"

namespace url {

class URL : public IUrl {
 public:
  URL(const std::string& url, std::shared_ptr<http::IHttpClient> http_client);
  ~URL();
  std::optional<http::HttpResponse> request();
  void show(std::string& body);

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

  Logger* logger;
  std::shared_ptr<http::IHttpClient> m_http_client;
  std::string m_url{};

  bool is_scheme_in(Scheme scheme) const;
  bool is_scheme_in(const std::vector<Scheme>& schemes) const;
};
}  // namespace url
