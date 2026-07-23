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
#include <magic_enum/magic_enum.hpp>
#include "logger.hpp"
#include "utils/utils.hpp"
using namespace my_b;
namespace my_b::url {

URL::URL(std::string_view _url) : url{_url} {
  // Trim url
  utils::trim(url);

  auto s1 = url.find("://");

  std::string raw_scheme = url.substr(0, s1);

  if (raw_scheme == "http") {
    scheme = Scheme::HTTP;
  } else if (raw_scheme == "https") {
    scheme = Scheme::HTTPS;
  } else if (raw_scheme == "file") {
    scheme = Scheme::FILE;
  } else {
    auto s = url.find(':');
    if (s != std::string::npos) {
      raw_scheme = url.substr(0, s);
      if (raw_scheme == "data") {
        scheme = Scheme::DATA;
      } else if (raw_scheme == "view-source") {
        scheme = Scheme::VIEW_SOURCE;
        url = url.substr(s + 1);
      } else {
        logger.err("Unsupported URL scheme: {}", magic_enum::enum_name(scheme));
      }
    } else {
      logger.err("Invalid URL");
    }
  }

  std::string rest = url.substr(s1 + 3);

  if (is_scheme_in(Scheme::FILE)) {
    url = rest;
  }

  if (is_scheme_in(Scheme::DATA)) {
    auto s1 = url.find(':');
    auto rest = url.substr(s1 + 1);
    auto s2 = rest.find(',');
    data_scheme.protocol = rest.substr(0, s2);
    data_scheme.data = rest.substr(s2 + 1);
  }
}

URL::~URL() = default;

bool URL::is_scheme_in(Scheme s) const { return scheme == s; }

bool URL::is_scheme_in(const std::vector<Scheme>& ss) const {
  for (const auto& s : ss) {
    if (scheme == s) {
      return true;
    }
  }
  return false;
}

}  // namespace my_b::url
