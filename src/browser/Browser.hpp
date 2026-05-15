#pragma once
#include <SFML/Graphics.hpp>
#include <common.hpp>
#include <functional>
#include <initializer_list>
#include <unordered_map>
#include <vector>
#include "SFML/Graphics/Font.hpp"
#include "SFML/Window/Event.hpp"
#include "logger.hpp"
#include "ui/Scrollbar.hpp"
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
  void relayout_for_current_window_width();
  void register_event_handlers();
  void register_callback(sf::Event::EventType event, const EventCallback& cb);
  void register_callback(std::initializer_list<sf::Event::EventType> events,
                         const EventCallback& cb);
  void dispatch_event(const sf::Event& event);
  void update_ui_elements();

  sf::RenderWindow& m_window;
  sf::Font m_font;
  std::vector<common::PositionTextPair> m_display_list;
  std::string m_text_content;
  bool m_running{};

  Logger& logger = Logger::getInstance();
  ui::ScrollBar m_scroll_bar;

  std::unordered_map<sf::Event::EventType, std::vector<EventCallback>>
      m_event_callbacks;
};
}  // namespace browser
