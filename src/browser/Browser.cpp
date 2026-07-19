#include "Browser.hpp"
#include <fmt/base.h>
#include <fmt/core.h>
#include <openssl/evp.h>
#include <SFML/Graphics.hpp>
#include <utility>
#include <vector>
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"
#include "SFML/Window/Event.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
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
  m_display_content =
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
  sf::Clock deltaClock;
  sf::Clock fpsClock;
  unsigned int frameCount = 0;
  float currentFPS = 0.0f;
  while (m_running && m_window.isOpen()) {
    sf::Event event;
    while (m_window.pollEvent(event)) {
#ifdef DEBUG
      ImGui::SFML::ProcessEvent(m_window, event);
#endif
      dispatch_event(event);
    }

#ifdef DEBUG
    auto dt = deltaClock.restart();
    ImGui::SFML::Update(m_window, dt);
    frameCount++;
    if (fpsClock.getElapsedTime().asSeconds() >= 0.5f) {
      currentFPS = frameCount / fpsClock.restart().asSeconds();
      frameCount = 0;
    }
    // draw using imgui
    ImGui::Begin("Performance Statistics");
    ImGui::Text("FPS: %.0f", currentFPS);
    ImGui::Text("Frame Time: %.2d ms", dt.asMilliseconds());
    ImGui::End();
#endif
    m_window.clear(sf::Color::White);
    update_ui_elements();
    draw();
#ifdef DEBUG
    ImGui::SFML::Render(m_window);
#endif
    m_window.display();
  }
  ImGui::SFML::Shutdown();
}

void Browser::draw() {
  const auto scroll_pos = m_scroll_bar.get_current_roll_pos();

#ifdef DEBUG
  const sf::Vector2i mouse_pos = sf::Mouse::getPosition(m_window);
  ImGui::Begin("Debug Info");
  ImGui::Text("Window size: %d x %d", m_window.getSize().x,
              m_window.getSize().y);
  ImGui::Text("Scroll pos: %d", scroll_pos);
  ImGui::Text("Mouse pos: (%d, %d)", mouse_pos.x, mouse_pos.y);
  ImGui::End();
  ImDrawList* draw_list = ImGui::GetForegroundDrawList();
  // draw vertical line of the middle of window
  // draw_list->AddLine(ImVec2(m_window.getSize().x / 2, 0),
  //                    ImVec2(m_window.getSize().x / 2, m_window.getSize().y),
  //                    IM_COL32(0, 0, 255, 255), 1.0f);
#endif
  for (auto& [x, y, element, text] : m_display_content) {
    if (y > scroll_pos + m_window.getSize().y) {
      continue;
    }
    if (y + layout::VSTEP < scroll_pos) {
      continue;
    }

    if (element.type == layout::LayoutElementType::Text) {
      text.setPosition(x, y - scroll_pos);
      m_window.draw(text);
#ifdef DEBUG
      const sf::FloatRect bounds = text.getGlobalBounds();
      // auto front = std::get<3>(m_display_content.front());
      // auto back = std::get<3>(m_display_content.back());
      // auto line_width = back.getGlobalBounds().left +
      //                   back.getGlobalBounds().width - front.getPosition().x;
      // auto line_widht_middle = line_width / 2.0f + front.getPosition().x;
      // // draw line width
      // draw_list->AddLine(ImVec2(line_widht_middle, 0),
      //                    ImVec2(line_widht_middle, m_window.getSize().y),
      //                    IM_COL32(255, 100, 132, 255));
      //
      // draw_list->AddRect(
      //     ImVec2(front.getPosition().x, front.getPosition().y),
      //     ImVec2(back.getPosition().x + back.getGlobalBounds().width,
      //            back.getPosition().y + back.getGlobalBounds().height),
      //     IM_COL32(0, 255, 0, 255), 0.0f, 0, 1.5f);
      //
      // draw_list->AddLine(ImVec2(0, front.getPosition().y),
      //                    ImVec2(front.getPosition().x,
      //                    front.getPosition().y), IM_COL32(255, 0, 132, 255));
      // draw_list->AddLine(
      //     ImVec2(back.getGlobalBounds().left + back.getGlobalBounds().width,
      //            back.getPosition().y),
      //     ImVec2(m_window.getSize().x, back.getPosition().y),
      //     IM_COL32(255, 0, 132, 255));
      //
      if (bounds.contains(static_cast<sf::Vector2f>(mouse_pos))) {
        draw_list->AddRect(
            ImVec2(bounds.left, bounds.top),
            ImVec2(bounds.left + bounds.width, bounds.top + bounds.height),
            IM_COL32(255, 0, 0, 255), 0.0f, 0, 1.5f);

        ImGui::SetTooltip(
            "Text: \"%s\"\nPos: (%.1f, %.1f)\nSize: %.1f x %.1f\nFont size: %i",
            text.getString().toAnsiString().c_str(), bounds.left, bounds.top,
            bounds.width, bounds.height, text.getCharacterSize());
      }
#endif
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
  if (!m_display_content.empty()) {
    m_scroll_bar.update(std::get<1>(m_display_content.back()),
                        static_cast<int>(m_window.getSize().y));
  }
}

void Browser::relayout_for_current_window_width() {
  m_display_content =
      layout::compute(m_text_content, m_font, m_window.getSize().x);
}

}  // namespace browser
