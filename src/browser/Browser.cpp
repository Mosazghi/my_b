#include "Browser.hpp"
#include <SFML/Graphics.hpp>
#include <sstream>
#include <utility>
#include <vector>
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"
#include "SFML/Window/Event.hpp"
#include "const.hpp"
#include "texture-manager/TextureManager.h"
#include "url/Url.hpp"
namespace browser {

Browser::Browser(sf::RenderWindow& window)
    : m_window{window}, m_running{true}, m_scroll_bar{window} {
  if (!m_font.loadFromFile("assets/RobotoMono-Regular.ttf")) {
    logger.err("Error loading font\n");
    return;
  }

  register_event_handlers();
}

void Browser::load(url::URL& url) {
  auto resp = url.request();
  m_text_content = common::lex(resp.response.body);
  m_display_list = common::layout(m_text_content, m_window.getSize().x);
}

void Browser::register_event_handlers() {
  register_callback(sf::Event::EventType::Closed, [&](const sf::Event&) {
    m_running = false;
    m_window.close();
  });

  register_callback(sf::Event::EventType::KeyPressed, [&](const sf::Event& e) {
    if (e.key.code == sf::Keyboard::Escape) {
      m_running = false;
      m_window.close();
    }
  });

  register_callback(sf::Event::EventType::Resized, [&](const sf::Event& e) {
    sf::FloatRect visibleArea(0, 0, e.size.width, e.size.height);
    m_window.setView(sf::View(visibleArea));
    relayout_for_current_window_width();
  });

  register_callback(sf::Event::EventType::MouseWheelScrolled,
                    [&](const sf::Event& e) { m_scroll_bar.mouse_scroll(e); });
  register_callback(
      {sf::Event::EventType::MouseMoved,
       sf::Event::EventType::MouseButtonPressed,
       sf::Event::EventType::MouseButtonReleased},
      [&](const sf::Event& e) { m_scroll_bar.mouse_hold_scroll(e); });

  register_callback(
      sf::Event::EventType::MouseButtonPressed,
      [&](const sf::Event& e) { m_scroll_bar.mouse_click_scroll(e); });
}

void Browser::register_callback(sf::Event::EventType event,
                                const EventCallback& cb) {
  m_event_callbacks[event].push_back(cb);
}

void Browser::register_callback(
    std::initializer_list<sf::Event::EventType> events,
    const EventCallback& cb) {
  for (const auto& event : events) {
    m_event_callbacks[event].push_back(cb);
  }
}

void Browser::dispatch_event(const sf::Event& event) {
  auto it = m_event_callbacks.find(event.type);
  if (it == m_event_callbacks.end()) {
    return;
  }
  for (const auto& cb : it->second) {
    cb(event);
  }
}

void Browser::spin() {
  while (m_running && m_window.isOpen()) {
    sf::Event event;
    while (m_window.pollEvent(event)) {
      dispatch_event(event);
    }

    m_window.clear();
    update_ui_elements();
    draw();
    m_window.display();
  }
}

void Browser::draw() {
  constexpr auto scroll_bar_width{20};
  const auto scroll_pos = m_scroll_bar.get_scroll_pos();
  for (const auto& [x, y, type] : m_display_list) {
    if (y > static_cast<int>(scroll_pos + m_window.getSize().y)) {
      continue;
    }
    if (y + consts::VSTEP < scroll_pos) {
      continue;
    }

    if (type.type == common::TextureType::TEXT) {
      sf::String glyph(type.value);
      sf::Text character(glyph, m_font, 12);
      character.setFillColor(sf::Color::White);
      character.setPosition(x - scroll_bar_width, y - scroll_pos);
      m_window.draw(character);
    } else {
      std::string id = common::get_emoji_id(type.value);
      auto texture = texture::TextureManager::get(id);
      if (!texture.has_value()) {
        logger.warn("Texture not found for codepoint: U+{}", id);
        continue;
      }
      sf::Sprite emoji(*texture);
      emoji.setPosition(x - scroll_bar_width, y - scroll_pos);
      emoji.setScale(0.8f, 0.8f);
      m_window.draw(emoji);
    }
  }
  m_scroll_bar.draw();
}

void Browser::update_ui_elements() {
  if (!m_display_list.empty()) {
    m_scroll_bar.update(std::get<1>(m_display_list.back()),
                        static_cast<int>(m_window.getSize().y));
  }
}

void Browser::relayout_for_current_window_width() {
  m_display_list = common::layout(m_text_content, m_window.getSize().x);
}

}  // namespace browser
