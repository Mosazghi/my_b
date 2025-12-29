#pragma once
#include <netdb.h>
#include <openssl/types.h>
#include <format>
#include <optional>
#include <string>
#include <unordered_map>
#include "IHttpClient.h"
#include "Types.h"
#include "logger.h"

namespace http {

static constexpr uint8_t MAX_CONSECUTIVE_REDIRS{5};
class HttpClient : public IHttpClient {
 public:
  HttpClient();
  ~HttpClient();

  /**
   * @brief Perform a HTTP GET request
   * @param url URL to request
   * @return std::optional<HttpResponse> HTTP response if successful,
   */
  std::optional<HttpResponse> get(std::string_view url) override;

 private:
  Logger& logger = Logger::getInstance();

  std::unordered_map<std::string, std::pair<int, addrinfo*>> m_http_sockets;
  std::unordered_map<std::string, std::pair<BIO*, SSL_CTX*>> m_https_sockets{};

  std::unordered_map<std::string, HttpRespCache> m_response_cache;

  // TODO: Refactor redirect logics
  bool m_last_redirect{};  //< Last request was a redirect
  HttpReqParams m_last_params{};
  int m_redirect_counts{};

  /**
   * @brief Gets HTTP status code from header
   * @param header HTTP header string
   * @return uint16_t HTTP status code
   */
  uint16_t get_status_code(const std::string& header) const;
  /**
   * @brief Get content length from HTTP header
   * @param header HTTP header string
   * @return uint16_t Content length, 0 if not found
   */
  uint16_t get_content_len(const std::string& header) const;
  /**
   * @brief Read HTTP header and body from stream
   * @param func  Read function
   * @param stream  Stream to read from
   * @return  std::pair<std::string, std::string> Pair of header and body
   * strings
   */
  template <typename ReadFunc, typename Stream>
  std::pair<std::string, std::string> get_header_body(ReadFunc func,
                                                      Stream stream) const;

  /**
   * @brief Perform HTTP request
   * @param params  HTTP request parameters
   * @param buffer  Request buffer
   * @return  std::optional<HttpResponse> HTTP response if successful,
   */
  std::optional<HttpResponse> http_req(const HttpReqParams& params,
                                       std::string_view buffer);
  /**
   * @brief Perform HTTPS request
   * @param params  HTTP request parameters
   * @param buffer  Request buffe
   * @return  std::optional<HttpResponse> HTTP response if successful,
   */
  std::optional<HttpResponse> https_req(HttpReqParams params,
                                        std::string_view buffer);

  /**
   * @brief Extract HTTP request parameters from URL
   * @param url URL string
   * @return  std::optional<HttpReqParams> HTTP request parameters if
   * successful,
   */
  std::optional<http::HttpReqParams> get_params_from_url(
      std::string_view url) const;

  /**
   * @brief Generate a cache key from request parameters
   * @param p HTTP request parameters
   * @return std::string Cache key
   */
  inline std::string get_cache_key(const HttpReqParams& p) const {
    return std::format("{}:{}", p.hostname, p.port);
  }

  /**
   * @brief Determine if the response is a redirect
   * @param r  HTTP response
   * @return  true if redirect, false otherwise
   */
  inline bool should_redirect(const HttpResponse& r) const {
    return (r.code >= 300 && r.code <= 399);
  }
};

}  // namespace http
