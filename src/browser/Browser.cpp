#include "Browser.hpp"
#include <openssl/evp.h>
#include <SFML/Graphics.hpp>
#include <utility>
#include <vector>
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"
#include "SFML/Window/Event.hpp"
#include "layout/layout.hpp"
#include "resource-manager/ResourceManager.h"
#include "ui/Scrollbar.hpp"
#include "url/Url.hpp"
namespace browser {

Browser::Browser(sf::RenderWindow& window)
    : m_window{window}, m_running{true}, m_scroll_bar{window} {
  if (!m_font.loadFromFile("assets/NotoSans-Regular.ttf")) {
    logger.err("Error loading font\n");
    return;
  }

  register_event_handlers();
}

void Browser::load(url::URL& url) {
  auto resp = url.request();
  m_text_content = common::lex(resp.response.body);
  m_display_list =
      layout::compute(m_text_content, m_font, m_window.getSize().x);
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
      {
          sf::Event::EventType::MouseMoved,
          sf::Event::EventType::MouseButtonPressed,
          sf::Event::EventType::MouseButtonReleased,
      },
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

    m_window.clear(sf::Color::White);
    update_ui_elements();
    draw();
    m_window.display();
  }
}

void Browser::draw() {
  const auto scroll_pos = m_scroll_bar.get_current_roll_pos();
  for (const auto& [x, y, element, text] : m_display_list) {
    if (y > scroll_pos + m_window.getSize().y) {
      continue;
    }
    if (y + layout::VSTEP < scroll_pos) {
      continue;
    }

    if (element.type == layout::LayoutElementType::TEXT) {
      sf::String glyph(element.value);
      sf::Text character(glyph, m_font, text.getCharacterSize());
      character.setStyle(text.getStyle());
      character.setFillColor(sf::Color::Black);
      const auto bounds = character.getLocalBounds();
      character.setOrigin(bounds.left, bounds.top);
      character.setPosition(x, y - scroll_pos);
      m_window.draw(character);
    } else {
      std::string id = common::get_emoji_id(element.value[0]);
      auto texture = resource::ResourceManager::get_texture(id);
      if (!texture.has_value()) {
        logger.warn("Texture not found for codepoint: U+{}", id);
        continue;
      }
      sf::Sprite emoji(*texture);
      const auto target_size = static_cast<float>(text.getCharacterSize());
      const auto tex_size = (*texture).getSize();
      const auto scale = target_size / static_cast<float>(tex_size.y);
      emoji.setScale(scale, scale);

      emoji.setPosition(x, y - scroll_pos);
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
  m_display_list =
      layout::compute(m_text_content, m_font, m_window.getSize().x);
}

}  // namespace browser
