#include "Scrollbar.hpp"
#include <fmt/base.h>
#include <algorithm>
#include "SFML/Graphics/Color.hpp"

using namespace my_b;
namespace my_b::ui {

void ScrollBar::update(int content_height, int viewport_height,
                       const sf::Vector2i& mouse_pos,
                       const sf::Vector2u& windowSize) {
  m_state.content_height = content_height;
  m_state.viewport_height = viewport_height;
  update_geometry(mouse_pos, windowSize);
}

void ScrollBar::update_geometry(const sf::Vector2i& mouse_pos,
                                const sf::Vector2u& windowSize) {
  const auto content_h = static_cast<float>(m_state.content_height);
  const auto viewport_h = static_cast<float>(m_state.viewport_height);
  const auto scroll_pos = static_cast<float>(m_state.scroll_pos);

  if (content_h <= viewport_h) {
    return;
  }

  m_container.setSize(sf::Vector2f{SCROLL_BAR_WIDTH, viewport_h});
  m_container.setPosition(
      sf::Vector2f{static_cast<float>(windowSize.x) - SCROLL_BAR_WIDTH, 0.f});
  m_container.setFillColor(sf::Color::Transparent);

  const auto thumb_h = viewport_h * (viewport_h / content_h);
  const auto thumb_y = scroll_pos * (viewport_h / content_h);

  m_thumb.setSize(sf::Vector2f{SCROLL_BAR_WIDTH, thumb_h});
  m_thumb.setPosition(sf::Vector2f{
      static_cast<float>(windowSize.x) - SCROLL_BAR_WIDTH, thumb_y});

  m_state.is_hovering_thumb =
      m_thumb.getGlobalBounds().contains(mouse_pos.x, mouse_pos.y);

  m_thumb.setFillColor(m_state.is_hovering_thumb ? sf::Color(180, 180, 180)
                                                 : sf::Color(192, 192, 192));
  m_state.is_hovering_container =
      m_container.getGlobalBounds().contains(mouse_pos.x, mouse_pos.y);
  m_container.setFillColor(m_state.is_hovering_container
                               ? sf::Color(220, 220, 220)
                               : sf::Color::Transparent);
}

void ScrollBar::draw(sf::RenderTarget& target, sf::RenderStates states) const {
  if (static_cast<float>(m_state.content_height) <=
      static_cast<float>(m_state.viewport_height)) {
    return;
  }

  if (m_state.is_hovering_container) {
    target.draw(m_container, states);
  }

  bool should_hide =
      m_state.last_scroll_time + std::chrono::milliseconds(1500) <
      std::chrono::steady_clock::now();

  if (should_hide && !m_state.is_hovering_thumb) {
    return;
  }

  // Viktig: Bruk transformasjonen slik at setPosition() fungerer utenfra
  states.transform *= getTransform();
  target.draw(m_thumb, states);
}

void ScrollBar::mouse_click_scroll(const sf::Event& /*event*/,
                                   const sf::Vector2i& mouse_pos) {
  if (!m_container.getGlobalBounds().contains(mouse_pos.x, mouse_pos.y) ||
      m_thumb.getGlobalBounds().contains(mouse_pos.x, mouse_pos.y)) {
    return;
  }

  set_scroll_pos(
      get_scroll_pos_from_mouse(static_cast<sf::Vector2f>(mouse_pos)),
      mouse_pos);
}

void ScrollBar::mouse_hold_scroll(const sf::Event& event,
                                  const sf::Vector2i& mouse_pos) {
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
    set_scroll_pos(
        get_scroll_pos_from_mouse(static_cast<sf::Vector2f>(mouse_pos)),
        mouse_pos);
  }
}

void ScrollBar::set_scroll_pos(float pos, const sf::Vector2i& mouse_pos) {
  m_state.scroll_pos = pos;
  m_state.last_scroll_time = std::chrono::steady_clock::now();
  // update_geometry(mouse_pos, m_window.getSize());
}

float ScrollBar::get_scroll_pos_from_mouse(
    const sf::Vector2f& local_mouse) const {
  const float track_top = m_container.getPosition().y;
  const float track_height = m_container.getSize().y;
  const float thumb_height = m_thumb.getSize().y;

  const float max_scroll = std::max(
      0.f, (float)m_state.content_height - (float)m_state.viewport_height);
  const float track_range = std::max(1.f, track_height - thumb_height);

  float t = (local_mouse.y - track_top - thumb_height * 0.5f) / track_range;
  t = std::clamp(t, 0.f, 1.f);
  return t * max_scroll;
}

void ScrollBar::mouse_scroll(const sf::Event& event,
                             const sf::Vector2i& mouse_pos) {
  constexpr int scroll_step = 100;
  ScrollDirection direction = event.mouseWheelScroll.delta > 0
                                  ? ScrollDirection::UP
                                  : ScrollDirection::DOWN;

  const auto scroll_pos = m_state.scroll_pos;
  if (direction == ScrollDirection::UP) {
    set_scroll_pos(std::max(0.f, static_cast<float>(scroll_pos - scroll_step)),
                   mouse_pos);
  } else {
    int max_scroll =
        std::max(0, m_state.content_height - m_state.viewport_height);
    if (scroll_pos >= max_scroll) {
      return;
    }
    set_scroll_pos(std::min(static_cast<float>(max_scroll),
                            static_cast<float>(scroll_pos + scroll_step)),
                   mouse_pos);
  }
}

}  // namespace my_b::ui
