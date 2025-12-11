#pragma once
#include <url/Types.h>
#include <string>
#include <unordered_map>

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
  std::unordered_map<std::string, std::string> headers;
};

}  // namespace http
