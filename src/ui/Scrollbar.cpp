#include "Scrollbar.hpp"
#include <algorithm>
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Window/Mouse.hpp"

namespace ui {
ScrollBar::ScrollBar(sf::RenderWindow& window) : m_window{window} {}

void ScrollBar::update(int content_height, int viewport_height) {
  m_state.content_height = content_height;
  m_state.viewport_height = viewport_height;
}

void ScrollBar::draw() {
  constexpr float width = 20.f;

  const float content_h = static_cast<float>(m_state.content_height);
  const float viewport_h = static_cast<float>(m_state.viewport_height);
  const float scroll_pos = static_cast<float>(m_state.scroll_pos);

  if (content_h <= viewport_h) {
    return;
  }

  // Container
  m_container.setSize(sf::Vector2f{width, viewport_h});
  m_container.setPosition(
      sf::Vector2f{static_cast<float>(m_window.getSize().x) - width, 0.f});
  m_container.setFillColor(sf::Color::Transparent);
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
  bool should_hide =
      m_state.last_scroll_time + std::chrono::milliseconds(1500) <
      std::chrono::steady_clock::now();
  if (should_hide && !hovered) {
    return;
  }

  m_thumb.setFillColor(hovered ? sf::Color::White : sf::Color(192, 192, 192));

  m_window.draw(m_thumb);
}

void ScrollBar::mouse_click_scroll(const sf::Event& /*event*/) {
  const auto& mouse_pos = sf::Mouse::getPosition(m_window);
  if (!m_container.getGlobalBounds().contains(mouse_pos.x,

                                              mouse_pos.y) ||
      m_thumb.getGlobalBounds().contains(mouse_pos.x, mouse_pos.y)) {
    return;
  }

  set_scroll_pos(get_scroll_pos_from_mouse(mouse_pos));
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
    set_scroll_pos(get_scroll_pos_from_mouse(mouse_pos));
  }
}
void ScrollBar::set_scroll_pos(float pos) {
  m_state.scroll_pos = pos;
  m_state.last_scroll_time = std::chrono::steady_clock::now();
}

float ScrollBar::get_scroll_pos_from_mouse(
    const sf::Vector2i& mouse_pos) const {
  const float track_top = m_container.getPosition().y;
  const float track_height = m_container.getSize().y;
  const float thumb_height = m_thumb.getSize().y;

  const float max_scroll = std::max(
      0.f, (float)m_state.content_height - (float)m_state.viewport_height);
  const float track_range = std::max(1.f, track_height - thumb_height);

  float t = ((float)mouse_pos.y - track_top - thumb_height * 0.5) / track_range;
  t = std::clamp(t, 0.f, 1.f);
  return t * max_scroll;
}

void ScrollBar::mouse_scroll(const sf::Event& event) {
  constexpr int scroll_step = 100;
  ScrollDirection direction = event.mouseWheelScroll.delta > 0
                                  ? ScrollDirection::UP
                                  : ScrollDirection::DOWN;

  const auto scroll_pos = m_state.scroll_pos;
  if (direction == ScrollDirection::UP) {
    set_scroll_pos(std::max(0, scroll_pos - scroll_step));
  } else {
    int max_scroll = std::max(
        0, m_state.content_height - static_cast<int>(m_window.getSize().y));
    if (scroll_pos >= max_scroll) {
      return;
    }

    set_scroll_pos(std::min(max_scroll, scroll_pos + scroll_step));
  }
}

}  // namespace ui
