#pragma once
#include <url/Types.h>
#include <chrono>
#include <string>
#include <unordered_map>

namespace http {
using Headers = std::unordered_map<std::string, std::string>;

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
  Headers headers;
};

struct HttpRespCache {
  std::string body;
  Headers headers;
  std::chrono::system_clock::time_point timestamp;
  uint32_t max_age;
};

}  // namespace http
