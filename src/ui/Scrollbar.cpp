#include "Scrollbar.hpp"
#include <algorithm>
#include <cstdio>
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Window/Mouse.hpp"

namespace ui {
ScrollBar::ScrollBar(sf::RenderWindow& window) : m_window{window} {}

void ScrollBar::update(int content_height, int viewport_height) {
  m_state.content_height = content_height;
  m_state.viewport_height = viewport_height;
  // scroll_pos is intentionally not touched — managed internally by
  // scrolldown()
}

void ScrollBar::draw() {
  constexpr float width = 20.f;

  const float content_h = static_cast<float>(m_state.content_height);
  const float viewport_h = static_cast<float>(m_state.viewport_height);
  const float scroll_pos = static_cast<float>(m_state.scroll_pos);

  if (content_h <= viewport_h) return;

  // Container
  m_container.setSize(sf::Vector2f{width, viewport_h});
  m_container.setPosition(
      sf::Vector2f{static_cast<float>(m_window.getSize().x) - width, 0.f});
  m_container.setFillColor(sf::Color(60, 60, 60));
  m_window.draw(m_container);

  // Thumb
  const float thumb_h = viewport_h * (viewport_h / content_h);
  const float thumb_y = scroll_pos * (viewport_h / content_h);

  m_thumb.setSize(sf::Vector2f{width, thumb_h});
  m_thumb.setPosition(
      sf::Vector2f{static_cast<float>(m_window.getSize().x) - width, thumb_y});

  const auto mouse_pos = sf::Mouse::getPosition(m_window);
  const bool hovered = m_thumb.getGlobalBounds().contains(sf::Vector2f{
      static_cast<float>(mouse_pos.x), static_cast<float>(mouse_pos.y)});
  m_thumb.setFillColor(hovered ? sf::Color::White : sf::Color(192, 192, 192));

  m_window.draw(m_thumb);
}

void ScrollBar::handleEvent(const sf::Event& /*event*/) {}

void ScrollBar::mouse_click_scroll(const sf::Event& e) {
  const auto mouse_pos = sf::Mouse::getPosition(m_window);
  const auto scroll_pos = m_state.scroll_pos;
  if (!m_container.getGlobalBounds().contains(mouse_pos.x,

                                              mouse_pos.y)) {
    return;
  }
  const auto delta_y = mouse_pos.y - scroll_pos;

  if (delta_y != 0) {
    const auto dir = delta_y > 0 ? ScrollDirection::DOWN : ScrollDirection::UP;
    const auto actual_scroll_step =
        std::abs(delta_y) + m_container.getSize().y / 2 * 2;
    scrolldown(dir, actual_scroll_step);
  }
}

void ScrollBar::mouse_hold_scroll(const sf::Event& event) {
  const auto mouse_pos = sf::Mouse::getPosition(m_window);
  static bool is_scrolling = false;
  if (event.type == sf::Event::MouseButtonPressed &&
      event.mouseButton.button == sf::Mouse::Left &&
      m_thumb.getGlobalBounds().contains(mouse_pos.x, mouse_pos.y)) {
    is_scrolling = true;
  } else if (event.type == sf::Event::MouseButtonReleased &&
             event.mouseButton.button == sf::Mouse::Left) {
    is_scrolling = false;
  }

  if (is_scrolling) {
    static int last_mouse_y = mouse_pos.y;
    int delta_y = mouse_pos.y - last_mouse_y;
    if (delta_y != 0) {
      ScrollDirection direction =
          delta_y > 0 ? ScrollDirection::DOWN : ScrollDirection::UP;
      scrolldown(direction);
      last_mouse_y = mouse_pos.y;
    }
  }
}

void ScrollBar::mouse_scroll(const sf::Event& event) {
  ScrollDirection dir = event.mouseWheelScroll.delta > 0
                            ? ScrollDirection::UP
                            : ScrollDirection::DOWN;

  scrolldown(dir);
}

void ScrollBar::scrolldown(ScrollDirection direction, const int scroll_step) {
  const auto scroll_pos = m_state.scroll_pos;
  if (direction == ScrollDirection::UP) {
    m_state.scroll_pos = std::max(0, scroll_pos - scroll_step);
  } else {
    int max_scroll = std::max(
        0, m_state.content_height - static_cast<int>(m_window.getSize().y));
    if (scroll_pos >= max_scroll) {
      return;
    }

    m_state.scroll_pos = std::min(max_scroll, scroll_pos + scroll_step);
  }
}

}  // namespace ui
