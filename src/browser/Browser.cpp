#include "Browser.hpp"
#include <SFML/Graphics.hpp>
#include <sstream>
#include <utility>
#include <vector>
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"
#include "SFML/Window/Event.hpp"
#include "const.hpp"
#include "texture-manager/TextureManager.h"
#include "url/Url.hpp"
namespace browser {

Browser::Browser(sf::RenderWindow& window) : m_window{window}, m_running{true} {
  if (!m_font.loadFromFile("/usr/share/fonts/hack/Hack-Italic.ttf")) {
    logger.err("Error loading font\n");
    m_running = false;
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
                    [&](const sf::Event& e) { mouse_scroll(e); });
  register_callback(sf::Event::EventType::MouseMoved,
                    [&](const sf::Event& e) { mouse_hold_scroll(e); });
  register_callback(sf::Event::EventType::MouseButtonPressed,
                    [&](const sf::Event& e) { mouse_hold_scroll(e); });
  register_callback(sf::Event::EventType::MouseButtonReleased,
                    [&](const sf::Event& e) { mouse_hold_scroll(e); });
}

void Browser::register_callback(sf::Event::EventType event, EventCallback cb) {
  m_event_callbacks[event].push_back(std::move(cb));
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
      std::cout << "Eventtype: " << event.type << '\n';
      dispatch_event(event);
    }

    m_window.clear();
    draw();
    m_window.display();
  }
}

void Browser::draw() {
  constexpr auto scroll_bar_width{20};
  // Text
  for (const auto& [x, y, type] : m_display_list) {
    if (y > static_cast<int>(m_scroll + m_window.getSize().y)) {
      continue;
    }
    if (y + consts::VSTEP < m_scroll) {
      continue;
    }

    if (type.type == common::TextureType::TEXT) {
      sf::String glyph(type.value);
      sf::Text character(glyph, m_font, 12);
      character.setFillColor(sf::Color::White);
      character.setPosition(x - scroll_bar_width, y - m_scroll);
      m_window.draw(character);
    } else {
      sf::String temp(type.value);
      std::string utf8_char = temp.toAnsiString();
      std::stringstream id_stream;
      id_stream << std::hex << std::uppercase << std::setfill('0')
                << std::setw(5) << static_cast<uint32_t>(type.value)
                << std::dec;

      std::string id = id_stream.str();
      auto texture = texture::TextureManager::get(id);
      sf::Sprite emoji(texture);
      emoji.setPosition(x - scroll_bar_width, y - m_scroll);
      emoji.setScale(0.8f, 0.8f);
      m_window.draw(emoji);
    }
  }
  float last_elem_y = std::get<1>(m_display_list.back());
  float window_y = m_window.getSize().y;
  if (last_elem_y <= window_y) {
    return;
  }
  // Scrollbar
  const int scroll_bar_height = window_y * (window_y / last_elem_y);
  const int scroll_bar_y = m_scroll * (window_y / last_elem_y);
  m_scroll_bar.setSize(sf::Vector2f(scroll_bar_width, scroll_bar_height));
  m_scroll_bar.setPosition(m_window.getSize().x - scroll_bar_width,
                           scroll_bar_y);

  const auto mouse_pos = sf::Mouse::getPosition(m_window);
  if (m_scroll_bar.getGlobalBounds().contains(mouse_pos.x, mouse_pos.y)) {
    m_scroll_bar.setFillColor(sf::Color::White);
  } else {
    m_scroll_bar.setFillColor(sf::Color(192, 192, 192));
  }
  m_window.draw(m_scroll_bar);
}

void Browser::relayout_for_current_window_width() {
  m_display_list = common::layout(m_text_content, m_window.getSize().x);
}

void Browser::mouse_hold_scroll(const sf::Event& event) {
  auto mouse_pos = sf::Mouse::getPosition(m_window);
  static bool is_scrolling = false;
  if (event.type == sf::Event::MouseButtonPressed &&
      event.mouseButton.button == sf::Mouse::Left &&
      m_scroll_bar.getGlobalBounds().contains(mouse_pos.x, mouse_pos.y)) {
    is_scrolling = true;
    logger.dbg("Scrolling Mouse Y: {}", mouse_pos.y);
  } else if (event.type == sf::Event::MouseButtonReleased &&
             event.mouseButton.button == sf::Mouse::Left) {
    is_scrolling = false;
    logger.dbg("Stopped Scrolling");
  }

  if (is_scrolling) {
    static int last_mouse_y = mouse_pos.y;
    int delta_y = mouse_pos.y - last_mouse_y;
    std::cout << "delta: \t" << delta_y << '\n';
    if (delta_y != 0) {
      ScrollDirection direction =
          delta_y > 0 ? ScrollDirection::DOWN : ScrollDirection::UP;
      scrolldown(direction);
      last_mouse_y = mouse_pos.y;
    }
  }
}

void Browser::mouse_scroll(const sf::Event& event) {
  ScrollDirection dir = event.mouseWheelScroll.delta > 0
                            ? ScrollDirection::UP
                            : ScrollDirection::DOWN;

  scrolldown(dir);
}

void Browser::scrolldown(ScrollDirection direction, const int scroll_step) {
  if (direction == ScrollDirection::UP) {
    m_scroll = std::max(0, m_scroll - scroll_step);
  } else if (direction == ScrollDirection::DOWN) {
    if (m_display_list.empty()) {
      return;
    }
    int last_elem_y = std::get<1>(m_display_list.back());
    int max_scroll =
        std::max(0, last_elem_y - static_cast<int>(m_window.getSize().y));
    if (m_scroll >= max_scroll) {
      return;
    }

    m_scroll = std::min(max_scroll, m_scroll + scroll_step);
  }
}

}  // namespace browser
