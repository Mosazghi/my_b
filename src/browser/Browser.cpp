#include "Browser.hpp"
#include <SFML/Graphics.hpp>
#include "url/Url.hpp"
namespace browser {

Browser::Browser(sf::RenderWindow& window)
    : m_window{window}, m_running{true} {}

void Browser::load(url::URL& url) {
  auto resp = url.request();
  // if (resp.is_success()) {
  //   // std::cout << "Response body:\n" << resp.response.body << '\n';
  // } else {
  //   // std::cout << "Failed to load URL: " << url.m_url << '\n';
  // }
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

  constexpr auto HSTEP = 13, VSTEP = 15;

  while (m_running && m_window.isOpen()) {
    sf::Event event;
    while (m_window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        m_running = false;
        m_window.close();
      }
      if (event.type == sf::Event::KeyPressed &&
          event.key.code == sf::Keyboard::Escape) {
        m_running = false;
        m_window.close();
      }
    }

    m_window.clear(sf::Color(30, 30, 50));

    auto cursor_x = HSTEP, cursor_y = VSTEP;
    for (const sf::Uint32 cp : decoded) {
      if (cp == U'\n') {
        cursor_x = HSTEP;
        cursor_y += VSTEP;
        continue;
      }
      if (cursor_x >= 1280 - HSTEP) {
        cursor_x = HSTEP;
        cursor_y += VSTEP;
      }
      sf::String glyph(cp);
      sf::Text ch(glyph, font, 12);
      ch.setFillColor(sf::Color::White);
      ch.setPosition(cursor_x, cursor_y);
      m_window.draw(ch);
      cursor_x += HSTEP;
    }

    m_window.display();
  }
}

}  // namespace browser
