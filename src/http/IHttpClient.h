#pragma once
#include <optional>
#include <string>
#include "Types.h"

namespace http {
class IHttpClient {
 public:
  virtual ~IHttpClient() {};
  virtual std::optional<http::HttpResponse> get(
      const std::string& url) const = 0;
};

}  // namespace http
