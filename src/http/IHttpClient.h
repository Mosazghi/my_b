#pragma once
#include <optional>
#include "Types.h"

namespace http {
class IHttpClient {
 public:
  virtual ~IHttpClient() {};
  virtual std::optional<http::HttpResponse> get(
      http::HttpReqParams p) const = 0;
};

}  // namespace http
