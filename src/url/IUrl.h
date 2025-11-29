#pragma once
#include <http/Types.h>
#include <optional>
#include <string>

namespace url {

class IUrl {
 public:
  virtual ~IUrl() {};
  virtual void show(const std::string& body) = 0;
  virtual std::optional<http::HttpResponse> request() = 0;
};

}  // namespace url
