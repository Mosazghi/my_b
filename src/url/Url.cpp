#include "url/Url.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cassert>
#include <http/HttpClient.hpp>
#include <http/IHttpClient.hpp>
#include <optional>
#include <utility>
#include "SFML/Window/Keyboard.hpp"
#include "file/File.hpp"
#include "logger.hpp"
#include "url/Types.hpp"
#include "utils.hpp"

namespace url {

URL::URL(std::string_view url, std::shared_ptr<http::IHttpClient> http_client)
    : m_http_client{std::move(std::move(http_client))}, m_url{url} {
  // Trim url
  utils::trim(m_url);

  auto s1 = m_url.find("://");

  std::string scheme = m_url.substr(0, s1);

  // Parse potential schemes
  if (scheme == "http") {
    m_data.scheme = Scheme::HTTP;
  } else if (scheme == "https") {
    m_data.scheme = Scheme::HTTPS;
  } else if (scheme == "file") {
    m_data.scheme = Scheme::FILE;
  } else {
    auto s = m_url.find(':');
    if (s != std::string::npos) {
      auto scheme = m_url.substr(0, s);
      if (scheme == "data") {
        m_data.scheme = Scheme::DATA;
      } else if (scheme == "view-source") {
        m_data.scheme = Scheme::VIEW_SOURCE;
        m_url = m_url.substr(s + 1);
      } else {
        logger.err("Unsupported URL scheme: {}", scheme);
      }
    } else {
      logger.err("Invalid URL");
    }
  }

  std::string rest = m_url.substr(s1 + 3);

  if (is_scheme_in(Scheme::FILE)) {
    m_url = rest;
  }

  if (is_scheme_in(Scheme::DATA)) {
    auto s1 = m_url.find(':');
    auto rest = m_url.substr(s1 + 1);
    auto s2 = rest.find(',');
    m_data.data_scheme.protocol = rest.substr(0, s2);
    m_data.data_scheme.data = rest.substr(s2 + 1);
    logger.dbg("DATA SCHEME: {} ({}) :::: {},  {}", m_url, rest,
               m_data.data_scheme.protocol, m_data.data_scheme.data);
  }
}

URL::~URL() = default;

// TODO: Refactor to include generics instead
http::HttpResult URL::request() {
  http::HttpResult resp{};

  const auto apply_about_blank_details = [](http::HttpResult& r) {
    r.response.status_line = http::HttpStatusLine{
        .version = "HTTP/1.1", .explanation = "OK", .status = 200};
    r.response.body = "<html><body><h1>About Blank</h1></body></html>";
    r.response.headers["content-type"] = "text/html";
  };

  switch (m_data.scheme) {
    case Scheme::HTTP:
    case Scheme::HTTPS:
    case Scheme::VIEW_SOURCE:
      resp = m_http_client->get(m_url);
      break;
    case Scheme::FILE: {
      auto file = file::read(m_url);
      if (file.has_value()) {
        resp.response.status_line = http::HttpStatusLine{
            .version = "HTTP/1.1", .explanation = "OK", .status = 200};
        resp.response.body = file.value();
      } else {
        resp.errors.emplace_back("File not found");
        resp.response.status_line = http::HttpStatusLine{
            .version = "HTTP/1.1", .explanation = "Not Found", .status = 404};
      }
      break;
    }
    case Scheme::DATA:
      // TODO: Add support for data scheme
    case Scheme::UNKNOWN:
    default:
      resp.errors.emplace_back("Unsupported URL scheme");
      apply_about_blank_details(resp);
      break;
  }
  return resp;
}

bool URL::is_scheme_in(Scheme s) const { return m_data.scheme == s; }

bool URL::is_scheme_in(const std::vector<Scheme>& ss) const {
  for (const auto& s : ss) {
    if (m_data.scheme == s) return true;
  }
  return false;
}

}  // namespace url
