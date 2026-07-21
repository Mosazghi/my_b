#pragma once
#include <string>
#include "logger.hpp"

namespace my_b::url {

enum class Scheme : std::uint8_t {
  UNKNOWN,  //< Unsupported.
  HTTP,
  HTTPS,
  FILE,
  DATA,
  VIEW_SOURCE
};

struct URL {
  URL(std::string_view url);
  ~URL();

  std::string url{};
  std::string host{};
  std::string path{};
  std::optional<int> port{};
  Scheme scheme{Scheme::UNKNOWN};
  // TODO: Refactor
  struct DataScheme {
    std::string protocol;
    std::string data;
  } data_scheme{};

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

 private:
  Logger& logger = Logger::getInstance();
};
}  // namespace my_b::url
