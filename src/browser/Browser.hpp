#pragma once
#include <SFML/Graphics.hpp>
#include <common.hpp>
#include "SFML/Graphics/Font.hpp"
#include "logger.hpp"
#include "url/Url.hpp"
namespace browser {

class Browser {
 public:
  explicit Browser(sf::RenderWindow& window);
  void load(url::URL& url);
  ~Browser() = default;
  void spin();
  void draw();

 private:
  int getMaxScroll() const;
  void relayoutForCurrentWindowWidth();
  void scrolldown(const sf::Event& event);

  sf::RenderWindow& m_window;
  sf::Font m_font;
  std::vector<common::PositionTextPair> m_display_list;
  std::string m_text_content;
  bool m_running{};
  int m_scroll{};
  sf::RectangleShape m_scroll_bar{};
  Logger& logger = Logger::getInstance();
};
}  // namespace browser
