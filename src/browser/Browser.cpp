#include "Browser.hpp"
#include <SFML/Graphics.hpp>
#include <sstream>
#include <vector>
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"
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

  m_scroll_bar.setFillColor(sf::Color::White);
}

void Browser::load(url::URL& url) {
  auto resp = url.request();
  m_text_content = common::lex(resp.response.body);
  m_display_list = common::layout(m_text_content, m_window.getSize().x);
}

void Browser::spin() {
  while (m_running && m_window.isOpen()) {
    sf::Event event;
    while (m_window.pollEvent(event)) {
      auto shouldClose = event.type == sf::Event::Closed ||
                         (event.type == sf::Event::KeyPressed &&
                          event.key.code == sf::Keyboard::Escape);

      if (shouldClose) {
        m_running = false;
        m_window.close();
      }

      if (event.type == sf::Event::Resized) {
        sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
        m_window.setView(sf::View(visibleArea));
        relayoutForCurrentWindowWidth();
      }

      if (event.type == sf::Event::MouseWheelScrolled ||
          (event.type == sf::Event::MouseButtonPressed &&
           event.mouseButton.button == sf::Mouse::Left)) {
        scrolldown(event);
      }
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
      sf::Text ch(glyph, m_font, 12);
      ch.setFillColor(sf::Color::White);
      ch.setPosition(x - scroll_bar_width, y - m_scroll);
      m_window.draw(ch);
    } else {
      // auto

      sf::String temp(type.value);
      std::string utf8_char = temp.toAnsiString();

      std::stringstream s;

      s << std::hex << std::uppercase << std::setfill('0') << std::setw(5)
        << static_cast<uint32_t>(type.value) << std::dec;

      std::string id = s.str();
      auto texture = texture::TextureManager::get(id);
      sf::Sprite emoji(texture);
      emoji.setPosition(x - scroll_bar_width, y - m_scroll);
      emoji.setScale(0.8f, 0.8f);
      m_window.draw(emoji);
    }
  }
  // Scrollbar container
  sf::RectangleShape scroll_bar_container(
      sf::Vector2f(scroll_bar_width, m_window.getSize().y));
  scroll_bar_container.setFillColor(sf::Color(128, 128, 128));
  scroll_bar_container.setPosition(m_window.getSize().x - scroll_bar_width, 0);
  m_window.draw(scroll_bar_container);

  // Scrollbar
  float last_elem_y = std::get<1>(m_display_list.back());
  float window_y = m_window.getSize().y;
  const int scroll_bar_height = window_y * (window_y / last_elem_y);
  const int scroll_bar_y = m_scroll * (window_y / last_elem_y);
  m_scroll_bar.setSize(sf::Vector2f(scroll_bar_width, scroll_bar_height));
  m_scroll_bar.setPosition(m_window.getSize().x - scroll_bar_width,
                           scroll_bar_y);
  m_window.draw(m_scroll_bar);
}

void Browser::relayoutForCurrentWindowWidth() {
  m_display_list = common::layout(m_text_content, m_window.getSize().x);
}

void Browser::scrolldown(const sf::Event& event) {
  constexpr auto SCROLL_STEP = 100;
  bool scrollUp = (event.type == sf::Event::MouseWheelScrolled &&
                   event.mouseWheelScroll.delta > 0) ||
                  (event.type == sf::Event::KeyPressed &&
                   event.key.code == sf::Keyboard::Up);
  bool scrollDown = (event.type == sf::Event::MouseWheelScrolled &&
                     event.mouseWheelScroll.delta < 0) ||
                    (event.type == sf::Event::KeyPressed &&
                     event.key.code == sf::Keyboard::Down);

  if (scrollUp) {
    m_scroll = std::max(0, m_scroll - SCROLL_STEP);
  } else if (scrollDown) {
    if (m_display_list.empty()) {
      return;
    }
    int last_elem_y = std::get<1>(m_display_list.back());
    int max_scroll =
        std::max(0, last_elem_y - static_cast<int>(m_window.getSize().y));
    if (m_scroll >= max_scroll) {
      return;
    }

    m_scroll = std::min(max_scroll, m_scroll + SCROLL_STEP);
  }
}

}  // namespace browser
