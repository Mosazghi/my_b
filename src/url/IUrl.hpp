#pragma once

#include "http/HttpRequest.hpp"
namespace url {

class IUrl {
 public:
  virtual ~IUrl() = default;
  virtual http::HttpResult request() = 0;
};

}  // namespace url
