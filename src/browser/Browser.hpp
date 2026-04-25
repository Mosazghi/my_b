#pragma once
#include <SFML/Graphics.hpp>
#include <common.hpp>
#include "logger.hpp"
#include "url/Url.hpp"
namespace browser {

class Browser {
 public:
  explicit Browser(sf::RenderWindow& window);
  void load(url::URL& url);
  ~Browser() = default;
  void spin();
  void draw(sf::Font& font);

 private:
  int getMaxScroll() const;
  void relayoutForCurrentWindowWidth();
  void scrolldown();

  sf::RenderWindow& m_window;
  std::vector<common::PositionTextPair> m_display_list;
  std::string m_text_content;
  bool m_running{};
  int m_scroll{};
  Logger& logger = Logger::getInstance();
};
}  // namespace browser
