#pragma once
#include <optional>
#include <string>
#include "Types.hpp"

namespace http {
class IHttpClient {
 public:
  virtual ~IHttpClient() {};
  virtual http::HttpResult get(std::string_view url) = 0;
};

}  // namespace http
