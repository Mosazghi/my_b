#include "Browser.hpp"
#include <SFML/Graphics.hpp>
#include "const.hpp"
#include "url/Url.hpp"
namespace browser {

Browser::Browser(sf::RenderWindow& window)
    : m_window{window}, m_running{true} {}

void Browser::load(url::URL& url) {
  auto resp = url.request();
  auto text = common::lex(resp.response.body);
  m_display_list = common::layout(text);
}

void Browser::spin(std::string body) {
  sf::Font font;
  if (!font.loadFromFile("/usr/share/fonts/google-noto-sans-mono-cjk-vf-fonts/"
                         "NotoSansMonoCJK-VF.ttc")) {
    logger.err("Error loading font\n");
    m_running = false;
    return;
  }

  // Decode the UTF-8 body to a sequence of Unicode codepoints (UTF-32)
  sf::String decoded = sf::String::fromUtf8(body.begin(), body.end());

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

      if (event.type == sf::Event::MouseWheelScrolled) {
        constexpr auto SCROLL_STEP = 100;
        if (event.mouseWheelScroll.delta > 0) {
          logger.inf("scrolled up");
          m_scroll = std::max(0, m_scroll - SCROLL_STEP);
        } else {
          logger.inf("scrolled down");
          m_scroll = std::max(0, m_scroll + SCROLL_STEP);
        }
      }
    }

    m_window.clear(sf::Color(30, 30, 50));
    this->draw(font);

    m_window.display();
  }
}

void Browser::draw(sf::Font& font) {
  for (const auto& [x, y, c] : m_display_list) {
    sf::String glyph(c);
    sf::Text ch(glyph, font, 12);
    ch.setFillColor(sf::Color::White);
    ch.setPosition(x, y - m_scroll);
    m_window.draw(ch);
  }
}

void Browser::scrolldown() {
  // constexpr auto SCROLL_STEP = 100;
  //
  // m_scroll += SCROLL_STEP;
  // this->draw();
}

}  // namespace browser
