#pragma once
#include <SFML/Graphics.hpp>
#include "logger.hpp"
#include "url/Url.hpp"
namespace browser {
class Browser {
 public:
  explicit Browser(sf::RenderWindow& window);
  void load(url::URL& url);
  ~Browser() = default;
  void spin(std::string body);

 private:
  sf::RenderWindow& m_window;
  bool m_running{};
  Logger& logger = Logger::getInstance();
};
}  // namespace browser
