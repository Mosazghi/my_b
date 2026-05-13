#pragma once
#include <memory>
#include <string>
#include "IUrl.hpp"
#include "http/HttpRequest.hpp"
#include "http/IHttpClient.hpp"
#include "logger.hpp"

namespace url {

class URL : public IUrl {
 public:
  URL(std::string_view url, std::shared_ptr<http::IHttpClient> http_client);
  ~URL() override;
  /**
   * @brief Perform the request for the URL
   * @return std::optional<http::HttpResponse> HTTP response if successful,
   * std::nullopt otherwise
   */
  http::HttpResult request() override;

 public:
  struct Data {
    Scheme scheme;
    std::string host;
    std::optional<int> port;
    std::string path;
    // TODO: Refactor
    struct DataScheme {
      std::string protocol;
      std::string data;
    } data_scheme;
  } m_data;

  Logger& logger = Logger::getInstance();
  std::shared_ptr<http::IHttpClient> m_http_client;
  std::string m_url{};

  /**
   * @brief Check if the URL scheme is in the given scheme
   * @param scheme Scheme to check
   * @return  bool True if the scheme matches, false otherwise
   */
  [[nodiscard]] bool is_scheme_in(Scheme s) const;

  /**
   * @brief Check if the URL scheme is in the given schemes
   * @param ss Vector of schemes to check
   * @return  bool True if the scheme matches any in the vector, false otherwise
   */
  [[nodiscard]] bool is_scheme_in(const std::vector<Scheme>& ss) const;
};
}  // namespace url
