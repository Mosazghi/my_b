#include "Browser.hpp"
#include <SFML/Graphics.hpp>
#include <cstdint>
#include "SFML/Graphics/View.hpp"
#include "const.hpp"
#include "url/Url.hpp"
namespace browser {

Browser::Browser(sf::RenderWindow& window)
    : m_window{window}, m_running{true} {}

void Browser::load(url::URL& url) {
  auto resp = url.request();
  m_text_content = common::lex(resp.response.body);
  m_display_list = common::layout(m_text_content, m_window.getSize().x);
}

void Browser::spin() {
  sf::Font font;
  if (!font.loadFromFile("/usr/share/fonts/google-noto-sans-mono-cjk-vf-fonts/"
                         "NotoSansMonoCJK-VF.ttc")) {
    logger.err("Error loading font\n");
    m_running = false;
    return;
  }

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
          event.type == sf::Event::KeyPressed) {
        scrolldown(event);
      }
    }

    m_window.clear(sf::Color(30, 30, 50));
    draw(font);

    m_window.display();
  }
}

void Browser::draw(sf::Font& font) {
  for (const auto& [x, y, c] : m_display_list) {
    if (y > static_cast<int>(m_scroll + m_window.getSize().y)) {
      continue;
    }
    if (y + consts::VSTEP < m_scroll) {
      continue;
    }

    sf::String glyph(c);
    sf::Text ch(glyph, font, 12);
    ch.setFillColor(sf::Color::White);
    ch.setPosition(x, y - m_scroll);
    m_window.draw(ch);
  }
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
