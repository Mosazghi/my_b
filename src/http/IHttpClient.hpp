#pragma once
#include "HttpRequest.hpp"

namespace my_b::http {
class IHttpClient {
 public:
  virtual ~IHttpClient() = default;
  virtual http::HttpResult get(std::string_view url) = 0;
};

}  // namespace my_b::http
