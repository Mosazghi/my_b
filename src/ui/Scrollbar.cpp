#include "Scrollbar.hpp"
#include <fmt/base.h>
#include <algorithm>
#include <iostream>
#include <magic_enum/magic_enum.hpp>
#include "SFML/Graphics/Color.hpp"

using namespace my_b;
namespace my_b::ui {

void ScrollBar::handle_event(const sf::Event& event, sf::RenderWindow& window) {
  const auto mouse_pos = sf::Mouse::getPosition(window);

  if (event.is<sf::Event::MouseWheelScrolled>()) {
    mouse_scroll(event);
  } else if (event.is<sf::Event::MouseMoved>() ||
             event.is<sf::Event::MouseButtonReleased>() ||
             event.is<sf::Event::MouseButtonPressed>()) {
    mouse_hold_scroll(event, mouse_pos);
  } else if (event.is<sf::Event::MouseButtonPressed>()) {
    mouse_click_scroll(event, mouse_pos);
  }

  update_geometry(mouse_pos);
}

void ScrollBar::update_geometry(const sf::Vector2i& mouse_pos) {
  const auto content_h = static_cast<float>(m_state.content_height);
  const auto viewport_h = static_cast<float>(m_state.viewport_height);
  const auto scroll_pos = static_cast<float>(m_state.scroll_pos);

  if (content_h <= viewport_h) {
    return;
  }

  m_container.setSize(sf::Vector2f{SCROLL_BAR_WIDTH, viewport_h});
  m_container.setPosition(
      sf::Vector2f{m_state.viewport_width - SCROLL_BAR_WIDTH, 0.f});
  m_container.setFillColor(sf::Color::Transparent);

  const auto thumb_h = viewport_h * (viewport_h / content_h);
  const auto thumb_y = scroll_pos * (viewport_h / content_h);

  m_thumb.setSize(sf::Vector2f{SCROLL_BAR_WIDTH, thumb_h});
  m_thumb.setPosition(
      sf::Vector2f{m_state.viewport_width - SCROLL_BAR_WIDTH, thumb_y});

  const auto mouse_pos_f = static_cast<sf::Vector2f>(mouse_pos);

  m_state.is_hovering_thumb = m_thumb.getGlobalBounds().contains(mouse_pos_f);

  m_thumb.setFillColor(m_state.is_hovering_thumb ? sf::Color(180, 180, 180)
                                                 : sf::Color(192, 192, 192));
  m_state.is_hovering_container =
      m_container.getGlobalBounds().contains(mouse_pos_f);
  m_container.setFillColor(m_state.is_hovering_container
                               ? sf::Color(220, 220, 220)
                               : sf::Color::Transparent);
}

void ScrollBar::draw(sf::RenderTarget& target, sf::RenderStates states) const {
  if (m_state.content_height <= m_state.viewport_height) {
    return;
  }

  states.transform *= getTransform();

  if (m_state.is_hovering_container) {
    target.draw(m_container, states);
  }

  bool should_hide_thumb =
      m_state.last_scroll_time + std::chrono::milliseconds(1500) <
      std::chrono::steady_clock::now();

  if (should_hide_thumb && !m_state.is_hovering_thumb) {
    return;
  }

  target.draw(m_thumb, states);
}

void ScrollBar::on_dimension_changed(const ui::ScrollDimensions& dims) {
  std::cout << "hmm" << std::endl;
  m_state.content_height = dims.content_height;
  m_state.viewport_height = dims.view_height;
  m_state.viewport_width = dims.view_width;
}

void ScrollBar::mouse_click_scroll(const sf::Event& /*event*/,
                                   const sf::Vector2i& mouse_pos) {
  if (!m_state.is_hovering_container || m_state.is_hovering_thumb) {
    return;
  }

  set_scroll_pos(get_scroll_pos_from_mouse(mouse_pos));
}

void ScrollBar::mouse_hold_scroll(const sf::Event& event,
                                  const sf::Vector2i& mouse_pos) {
  const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>();
  const auto* released = event.getIf<sf::Event::MouseButtonReleased>();

  if (pressed && pressed->button == sf::Mouse::Button::Left &&
      m_state.is_hovering_thumb) {
    m_state.is_dragging = true;
  } else if (released && released->button == sf::Mouse::Button::Left) {
    m_state.is_dragging = false;
  }

  if (m_state.is_dragging && event.is<sf::Event::MouseMoved>()) {
    set_scroll_pos(get_scroll_pos_from_mouse(mouse_pos));
  }
}

void ScrollBar::set_scroll_pos(float pos) {
  m_state.scroll_pos = pos;
  m_state.last_scroll_time = std::chrono::steady_clock::now();
}

float ScrollBar::get_scroll_pos_from_mouse(
    const sf::Vector2i& local_mouse) const {
  const float track_top = m_container.getPosition().y;
  const float track_height = m_container.getSize().y;
  const float thumb_height = m_thumb.getSize().y;

  const float max_scroll =
      std::max(0, m_state.content_height - m_state.viewport_height);
  const float track_range = std::max(1.f, track_height - thumb_height);

  float t = (local_mouse.y - track_top - thumb_height * 0.5f) / track_range;
  t = std::clamp(t, 0.f, 1.f);
  return t * max_scroll;
}

void ScrollBar::mouse_scroll(const sf::Event& event) {
  constexpr float scroll_sensitivity = 150.0f;
  const float delta = event.getIf<sf::Event::MouseWheelScrolled>()->delta;

  const float new_pos =
      static_cast<float>(m_state.scroll_pos) - (delta * scroll_sensitivity);
  const int max_scroll =
      std::max(0, m_state.content_height - m_state.viewport_height);

  set_scroll_pos(std::clamp(static_cast<int>(new_pos), 0, max_scroll));
}

}  // namespace my_b::ui
