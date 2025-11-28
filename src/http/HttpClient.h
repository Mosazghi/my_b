#pragma once
#include <optional>
#include <string>
#include "IHttpClient.h"
#include "Types.h"
#include "logger.h"
#include "url/Url.h"

// Forward declaration

namespace http {

class HttpClient : public IHttpClient {
 public:
  HttpClient();
  ~HttpClient();

  std::optional<HttpResponse> get(HttpReqParams params) const override;

 private:
  HttpResponse parse_response(const std::string& response) const;

  std::optional<HttpResponse> http_req(HttpReqParams params,
                                       const std::string& buffer) const;
  std::optional<HttpResponse> https_req(HttpReqParams params,
                                        const std::string& buffer) const;

  Logger* logger;
};

}  // namespace http
