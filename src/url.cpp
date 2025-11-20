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
namespace url {

// TODO: Improve parsing
URL::URL(const std::string& url, std::shared_ptr<http::HttpClient> http_client,
         std::shared_ptr<file::File> file_client)
    : m_http_client{http_client}, m_file_client(file_client) {
  logger = new Logger("URL");

  auto s1 = url.find("://");

  if (s1 == std::string::npos) {
    logger->err("Invalid url! [://] (scheme)");
    exit(EXIT_FAILURE);
  }
  std::string scheme = url.substr(0, s1);

  if (scheme == "http") {
    m_data.scheme = Scheme::HTTP;
  } else if (scheme == "https") {
    m_data.scheme = Scheme::HTTPS;
  } else if (scheme == "file") {
    m_data.scheme = Scheme::FILE;
  } else {
    logger->err("Other schemes are not yet supported!");
  }

  std::string rest = url.substr(s1 + 3);

  if (m_data.scheme == Scheme::FILE) {
    m_data.path = rest;
  } else {
    auto s2 = rest.find("/");

    if (s2 == std::string::npos) {
      m_data.path = "/";
      m_hostname = url.substr(s1 + 3);
    } else {
      m_hostname = rest.substr(0, s2);
      m_data.path = rest.substr(s2);
    }

    // Custom port
    auto c_port = m_hostname.find_first_of(":");
    if (c_port != std::string::npos) {
      m_data.port = std::stoi(m_hostname.substr(c_port + 1));
      m_hostname = m_hostname.substr(0, c_port);
    } else {
      m_data.port = m_data.scheme == Scheme::HTTP ? 80 : 443;
    }
  }
  logger->inf("Url init done");
}

URL::~URL() {}

std::optional<http::HttpResponse> URL::request() {
  std::optional<http::HttpResponse> resp{};

  switch (m_data.scheme) {
    case Scheme::HTTP:
    case Scheme::HTTPS:
      logger->inf("HTTP REQ");
      resp = m_http_client->get(
          {m_data.port.value(), m_hostname, m_data.path, m_data.scheme});
      break;
    case Scheme::FILE: {
      logger->inf("FILE REQ");
      auto file = m_file_client->read(m_data.path);
      if (file.has_value()) {
        resp = http::HttpResponse{.code = 200, .body = file.value()};
      } else {
        resp = http::HttpResponse{.code = 404, .body = "File not found"};
      }
      break;
    }
    default:
      break;
  }
  return resp;
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
}  // namespace url
