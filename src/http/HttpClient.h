#pragma once
#include <netdb.h>
#include <openssl/types.h>
#include <optional>
#include <string>
#include <unordered_map>
#include "IHttpClient.h"
#include "Types.h"
#include "logger.h"

namespace http {

class HttpClient : public IHttpClient {
 public:
  HttpClient();
  ~HttpClient();

  std::optional<HttpResponse> get(const std::string& url) override;

 private:
  static constexpr uint8_t MAX_CONSECUTIVE_REDIRS{5};

  uint16_t get_status_code(const std::string& header) const;
  uint16_t get_content_len(const std::string& header) const;

  template <typename ReadFunc, typename Stream>
  std::pair<std::string, std::string> get_header_body(ReadFunc func,
                                                      Stream stream) const;

  std::optional<HttpResponse> http_req(const HttpReqParams& params,
                                       const std::string& buffer);
  std::optional<HttpResponse> https_req(HttpReqParams params,
                                        const std::string& buffer);

  std::optional<http::HttpReqParams> get_params_from_url(
      const std::string& url) const;
  std::string get_cache_key(const HttpReqParams& params) const;
  bool should_redirect(const HttpResponse& r) const;
  Logger* logger;

  std::unordered_map<std::string, std::pair<int, addrinfo*>> m_http_sockets;
  std::unordered_map<std::string, std::pair<BIO*, SSL_CTX*>> m_https_sockets{};

  std::unordered_map<std::string, HttpRespCache> m_resp_cache;

  // TODO: Refactor redirect logics
  bool m_last_redirect{};  //< Last request was a reidrect
  HttpReqParams m_last_params{};
  int m_redirect_counts{};
};

}  // namespace http
