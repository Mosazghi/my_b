#include "url/Url.h"
#include <arpa/inet.h>
#include <assert.h>
#include <http/HttpClient.h>
#include <http/IHttpClient.h>
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
#include "file/File.h"
#include "logger.h"
namespace url {

// TODO: Improve parsing
URL::URL(const std::string& url, std::shared_ptr<http::IHttpClient> http_client)
    : m_http_client{http_client} {
  logger = new Logger("URL");

  auto s1 = url.find("://");

  std::string scheme = url.substr(0, s1);

  if (scheme == "http") {
    m_data.scheme = Scheme::HTTP;
  } else if (scheme == "https") {
    m_data.scheme = Scheme::HTTPS;
  } else if (scheme == "file") {
    m_data.scheme = Scheme::FILE;
  } else {
    // Check if it's 'data' scheme
    auto s = url.find(":");
    if (s != std::string::npos) {
      auto scheme = url.substr(0, s);
      if (scheme == "data") {
        m_data.scheme = Scheme::DATA;
      }
    }
  }

  std::string rest = url.substr(s1 + 3);

  if (is_scheme_in(Scheme::FILE)) {
    m_data.path = rest;
  } else if (is_scheme_in({Scheme::HTTP, Scheme::HTTPS})) {
    auto s2 = rest.find("/");

    if (s2 == std::string::npos) {
      m_data.path = "/";
      m_data.host = url.substr(s1 + 3);
    } else {
      m_data.host = rest.substr(0, s2);
      m_data.path = rest.substr(s2);
    }

    // Custom port
    auto c_port = m_data.host.find_first_of(":");
    if (c_port != std::string::npos) {
      m_data.port = std::stoi(m_data.host.substr(c_port + 1));
      m_data.host = m_data.host.substr(0, c_port);
    } else {
      m_data.port = m_data.scheme == Scheme::HTTP ? 80 : 443;
    }
  } else if (is_scheme_in(Scheme::DATA)) {
    auto s1 = url.find(":");
    auto rest = url.substr(s1 + 1);
    auto s2 = rest.find(",");
    m_data.data_scheme.protocol = rest.substr(0, s2);
    m_data.data_scheme.data = rest.substr(s2 + 1);
    logger->dbg("DATA SCHEME: {} ({}) :::: {},  {}", url, rest,
                m_data.data_scheme.protocol, m_data.data_scheme.data);
  }
}

URL::~URL() {}

std::optional<http::HttpResponse> URL::request() {
  std::optional<http::HttpResponse> resp{};

  switch (m_data.scheme) {
    case Scheme::HTTP:
    case Scheme::HTTPS:
      logger->inf("HTTP REQ");
      resp = m_http_client->get(http::HttpReqParams{
          m_data.port.value(), m_data.host, m_data.path, m_data.scheme});
      break;
    case Scheme::FILE: {
      logger->inf("FILE REQ");
      auto file = file::read(m_data.path);
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

bool URL::is_scheme_in(Scheme s) { return m_data.scheme == s; }

bool URL::is_scheme_in(const std::vector<Scheme>& ss) {
  for (const auto& s : ss) {
    if (m_data.scheme == s) {
      return true;
    }
  }
  return false;
}
}  // namespace url
