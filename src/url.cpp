#include "url.h"
#include <arpa/inet.h>
#include <assert.h>
#include <http_client.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <optional>
#include "logger.h"

URL::URL(const std::string& url, std::shared_ptr<http::HttpClient> http_client)
    : m_http_client{http_client} {
  logger = new Logger("URL");
  auto sep1 = url.find("://");
  if (sep1 == std::string::npos) {
    logger->err("Invalid url! [://]");
    exit(EXIT_FAILURE);
  }
  std::string scheme = url.substr(0, sep1);
  if (scheme == "http") {
    m_scheme = HttpScheme::HTTP;
  } else if (scheme == "https") {
    m_scheme = HttpScheme::HTTPS;
  } else {
    // logger->err("Other schemes are not yet supported!");
  }

  std::string rest = url.substr(sep1 + 3);

  if (rest[rest.size() - 1] != '/') {
    rest.append("/");
  }

  auto sep2 = rest.find("/");

  if (sep2 == std::string::npos) {
    logger->err("Invalid url! [/]");
    exit(EXIT_FAILURE);
  }

  m_hostname = rest.substr(0, sep2);
  m_path = rest.substr(sep2);
  // Custom port
  auto c_port = m_hostname.find_first_of(":");
  if (c_port != std::string::npos) {
    m_port = std::stoi(m_hostname.substr(c_port + 1));
    m_hostname = m_hostname.substr(0, c_port);
  } else {
    // FIXME: What if shceme is file or other?
    m_port = m_scheme == HttpScheme::HTTP ? 80 : 443;
  }
}

URL::~URL() {}

http::HttpResponse URL::request() {
  std::optional<http::HttpResponse> resp{};
  switch (m_scheme) {
    case HttpScheme::HTTP:
      resp = m_http_client->http_req({m_port, m_hostname, m_path});
      break;
    case HttpScheme::HTTPS:
      resp = m_http_client->https_req({m_port, m_hostname, m_path});
      break;
    default:
      break;
  }
  return resp.value_or(http::HttpResponse{});
}

void URL::show(const std::string& body) {
  bool in_tag{false};

  for (const auto& c : body) {
    if (c == '<') {
      in_tag = true;
    } else if (c == '>') {
      in_tag = false;
    } else if (!in_tag) {
      std::cout << c;
    }
  }
}
