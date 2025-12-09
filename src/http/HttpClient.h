#pragma once
#include <optional>
#include <string>
#include "IHttpClient.h"
#include "Types.h"
#include "logger.h"
#include "url/Url.h"

namespace http {

class HttpClient : public IHttpClient {
 public:
  HttpClient();
  ~HttpClient();

  std::optional<HttpResponse> get(const std::string& url) const override;

 private:
  int get_status_code(const std::string& response) const;

  std::optional<HttpResponse> http_req(HttpReqParams params,
                                       const std::string& buffer) const;
  std::optional<HttpResponse> https_req(HttpReqParams params,
                                        const std::string& buffer) const;

  std::optional<http::HttpReqParams> get_params_from_url(
      const std::string& url) const;
  Logger* logger;
};

}  // namespace http
