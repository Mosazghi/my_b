#pragma once
#include <SFML/Graphics.hpp>
#include <common.hpp>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>
#include "SFML/Graphics/Font.hpp"
#include "SFML/Window/Event.hpp"
#include "logger.hpp"
#include "url/Url.hpp"
namespace browser {

using EventCallback = std::function<void(const sf::Event&)>;
class Browser {
 public:
  explicit Browser(sf::RenderWindow& window);
  void load(url::URL& url);
  ~Browser() = default;
  void spin();
  void draw();

 private:
  int getMaxScroll() const;
  void relayout_for_current_window_width();
  void mouse_scroll(const sf::Event& event);
  enum class ScrollDirection : std::uint8_t { UP, DOWN };
  void scrolldown(ScrollDirection direction, const int scroll_step = 100);

  sf::RenderWindow& m_window;
  sf::Font m_font;
  std::vector<common::PositionTextPair> m_display_list;
  std::string m_text_content;
  bool m_running{};
  int m_scroll{};
  sf::RectangleShape m_scroll_bar{};
  Logger& logger = Logger::getInstance();

  void register_event_handlers();
  void register_callback(sf::Event::EventType event, EventCallback cb);
  void mouse_hold_scroll(const sf::Event& event);
  void dispatch_event(const sf::Event& event);
  std::unordered_map<sf::Event::EventType, std::vector<EventCallback>>
      m_event_callbacks;
};
}  // namespace browser
