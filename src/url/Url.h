#pragma once
#include <memory>
#include <string>
#include "IUrl.h"
#include "Types.h"
#include "http/IHttpClient.h"
#include "logger.h"

namespace url {

class URL : public IUrl {
 public:
  URL(std::string_view url, std::shared_ptr<http::IHttpClient> http_client);
  ~URL();
  /**
   * @brief Perform the request for the URL
   * @return std::optional<http::HttpResponse> HTTP response if successful,
   * std::nullopt otherwise
   */
  std::optional<http::HttpResponse> request();
  /**
   * @brief Show the body content
   * @param body Body content to show
   */
  void show(std::string& body);

 private:
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
  bool is_scheme_in(Scheme s) const { return m_data.scheme == s; }

  /**
   * @brief Check if the URL scheme is in the given schemes
   * @param ss Vector of schemes to check
   * @return  bool True if the scheme matches any in the vector, false otherwise
   */
  bool is_scheme_in(const std::vector<Scheme>& ss) const {
    for (const auto& s : ss) {
      if (m_data.scheme == s) {
        return true;
      }
    }
    return false;
  }
};
}  // namespace url
