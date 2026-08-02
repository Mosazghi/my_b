#include "Browser.hpp"
#include <fmt/base.h>
#include <fmt/core.h>
#include <openssl/evp.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include <memory>
#include <optional>
#include <utility>
#include <vector>
#include "../ui/Scrollbar.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"
#include "SFML/Window/Event.hpp"
#include "http/HttpClient.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
#include "layout/layout.hpp"
#include "resource-loader/ResourceLoader.hpp"
#include "resource-manager/ResourceManager.h"
#include "url/Url.hpp"
using namespace my_b;
namespace my_b::browser {

Browser::Browser(sf::RenderWindow& window)
    : m_running{true},
      m_http_client(std::make_shared<http::HttpClient>()),
      m_loader(
          std::make_unique<loader::ResourceLoader>(std::move(m_http_client))),
      m_window{window},
      m_ui_manager{m_window} {
  if (!m_font.openFromFile("assets/NotoSans-Regular.ttf")) {
    logger.err("Error loading font\n");
    return;
  }

  m_top_scrollbar = m_ui_manager.create_element<ui::ScrollBar>();
  subscribe_to_layout([&](const ui::ScrollDimensions& dims) {
    m_top_scrollbar->on_dimension_changed(dims);
  });
  register_event_handlers();
}

Browser::~Browser() = default;

void Browser::load(const url::URL& url) {
  auto resp = m_loader->load(url);
  m_text_content = common::lex(resp.response.body);
  m_display_content =
      layout::compute(m_text_content, m_font, m_window.getSize().x);
}

void Browser::register_event_handlers() {
  register_callback<sf::Event::Closed>([&](const sf::Event&) {
    m_running = false;
    m_window.close();
  });

  register_callback<sf::Event::KeyPressed>([&](const sf::Event& e) {
    if (e.getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape) {
      m_running = false;
      m_window.close();
    }
  });

  register_callback<sf::Event::Resized>([&](const sf::Event& e) {
    const auto size = e.getIf<sf::Event::Resized>()->size;
    const sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(size));
    m_window.setView(sf::View(visibleArea));
    relayout_for_current_window_width();
  });
}

void Browser::dispatch_event(const sf::Event& event) {
  const auto type =
      event.visit([](const auto& e) { return std::type_index(typeid(e)); });
  auto it = m_event_callbacks.find(type);
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
    while (const std::optional event = m_window.pollEvent()) {
#ifdef DEBUG
      ImGui::SFML::ProcessEvent(m_window, *event);
#endif
      dispatch_event(*event);
      m_ui_manager.handle_event(*event);
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
    draw();
#ifdef DEBUG
    ImGui::SFML::Render(m_window);
#endif
    m_window.display();
  }
  ImGui::SFML::Shutdown();
}

void Browser::draw() {
  static int scroll_pos = 0;
  if (m_top_scrollbar) {
    scroll_pos = m_top_scrollbar->get_current_roll_pos();
  }

#ifdef DEBUG
  const sf::Vector2i mouse_pos = sf::Mouse::getPosition(m_window);
  ImGui::Begin("Debug Info");
  ImGui::Text("Window size: %d x %d", m_window.getSize().x,
              m_window.getSize().y);
  ImGui::Text("Scroll pos: %d", scroll_pos);
  ImGui::Text("Mouse pos: (%d, %d)", mouse_pos.x, mouse_pos.y);
  ImGui::End();
  ImDrawList* draw_list = ImGui::GetForegroundDrawList();
#endif
  for (auto& [x, y, element, text] : m_display_content) {
    if (y > scroll_pos + m_window.getSize().y) {
      continue;
    }
    if (y + layout::VSTEP < scroll_pos) {
      continue;
    }

    const sf::Vector2f draw_pos{
        static_cast<float>(x),
        static_cast<float>(y) - static_cast<float>(scroll_pos)};

    if (element.type == layout::LayoutElementType::Text) {
      text.setPosition(draw_pos);
      m_window.draw(text);
#ifdef DEBUG
      const sf::FloatRect bounds = text.getGlobalBounds();
      if (bounds.contains(static_cast<sf::Vector2f>(mouse_pos))) {
        draw_list->AddRect(ImVec2(bounds.position.x, bounds.position.y),
                           ImVec2(bounds.position.x + bounds.size.x,
                                  bounds.position.y + bounds.size.y),
                           IM_COL32(255, 0, 0, 255), 0.0f, 0, 1.5f);

        ImGui::SetTooltip(
            "Text: \"%s\"\nPos: (%.1f, %.1f)\nSize: %.1f x %.1f\nFont size: %i",
            text.getString().toAnsiString().c_str(), bounds.position.x,
            bounds.position.y, bounds.size.x, bounds.size.y,
            text.getCharacterSize());
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
      emoji.setScale({scale, scale});

      emoji.setPosition(draw_pos);
      m_window.draw(emoji);
    }
  }

  m_ui_manager.draw(scroll_pos);
}

void Browser::relayout_for_current_window_width() {
  m_display_content =
      layout::compute(m_text_content, m_font, m_window.getSize().x);
  set_content_height(std::get<1>(m_display_content.back()));
  set_view_height(static_cast<float>(m_window.getSize().y));
  set_view_width(static_cast<float>(m_window.getSize().x));
}

}  // namespace my_b::browser
