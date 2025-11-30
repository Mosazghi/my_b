#pragma once
#include <url/Types.h>
#include <string>

namespace http {

enum class HttpPort {
  HTTP = 80,
  HTTPS = 443,
};

struct HttpReqParams {
  int port;
  std::string hostname;
  std::string path;
  url::Scheme scheme;
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

}  // namespace http
