#pragma once
#include <optional>
#include <string>
#include "logger.h"

namespace http {

enum class HttpPort {
  HTTP = 80,
  HTTPS = 443,
};

struct HttpReqParams {
  int port;
  std::string hostname;
  std::string path;
};

struct HttpStatusLine {
  std::string version;
  int status;
  std::string explaination;
};

struct HttpResponse {
  int code;
  std::string body;
};

class HttpClient {
 public:
  HttpClient();
  ~HttpClient();

  std::optional<HttpResponse> http_req(HttpReqParams params);
  std::optional<HttpResponse> https_req(HttpReqParams params);

 private:
  HttpResponse parse_response(const std::string& response);
  Logger* logger;
};

}  // namespace http
