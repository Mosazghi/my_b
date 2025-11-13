#pragma once
#include <memory>
#include <string>
#include "http_client.h"
#include "logger.h"

enum class HttpScheme { HTTP, HTTPS };

class URL {
 public:
  URL(std::string const& url, std::shared_ptr<HttpClient> http_client);
  ~URL();
  HttpResponse request();
  void show(const std::string& body);

 private:
  HttpScheme m_scheme{};
  std::string m_hostname{};
  std::string m_path{};
  int m_port;
  Logger* logger;
  std::shared_ptr<HttpClient> m_http_client;
};
