#pragma once
#include <string>
#include "logger.h"

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

  HttpResponse http_req(HttpReqParams params);
  HttpResponse https_req(HttpReqParams params);

 private:
  HttpResponse parse_response(const std::string& response);
  Logger* logger;
};
