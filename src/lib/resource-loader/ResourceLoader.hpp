#pragma once

#include <memory>
#include "http/IHttpClient.hpp"
#include "url/Url.hpp"
namespace my_b::loader {

class ResourceLoader {
 public:
  explicit ResourceLoader(std::shared_ptr<http::IHttpClient> client);
  http::HttpResult load(const url::URL& url);
  ~ResourceLoader();

 private:
  std::shared_ptr<http::IHttpClient> m_http_client{};
};
}  // namespace my_b::loader
