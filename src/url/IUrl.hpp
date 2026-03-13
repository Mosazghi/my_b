#pragma once
#include <http/Types.hpp>
#include <optional>
#include <string>

namespace url {

class IUrl {
 public:
  virtual ~IUrl() {};
  virtual http::HttpResult request() = 0;
};

}  // namespace url
