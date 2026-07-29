#pragma once
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Vector2.hpp>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Window/Event.hpp"
#include "UiElement.hpp"

namespace my_b::ui {

struct ScrollDimensions {
  float view_height;
  float content_height;
  float view_width;
};

struct ScrollState {
  bool is_hovering_thumb{false};
  bool is_hovering_container{false};
  bool is_dragging{false};
  int content_height{};
  int viewport_height{};
  int viewport_width{};
  int scroll_pos{};
  std::chrono::steady_clock::time_point last_scroll_time =
      std::chrono::steady_clock::now();
};

enum class ScrollDirection : std::uint8_t { UP, DOWN };

class ScrollBar : public UiElement {
  static constexpr float SCROLL_BAR_WIDTH{20.f};

 public:
  ScrollBar() = default;
  ~ScrollBar() override = default;

  void update(int content_height, int viewport_height,
              const sf::Vector2i& mouse_pos, const sf::Vector2u& windowSize);
  void handle_event(const sf::Event& event, sf::RenderWindow& window) override;

  [[nodiscard]] int get_current_roll_pos() const { return m_state.scroll_pos; }
  [[nodiscard]] float get_width() const { return SCROLL_BAR_WIDTH; }
  void on_dimension_changed(const ui::ScrollDimensions& dims);

 private:
  void mouse_click_scroll(const sf::Event& e, const sf::Vector2i& mouse_pos);
  void mouse_hold_scroll(const sf::Event& e, const sf::Vector2i& mouse_pos);
  void mouse_scroll(const sf::Event& e);
  void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
  [[nodiscard]] float get_scroll_pos_from_mouse(
      const sf::Vector2i& mouse_pos) const;
  void set_scroll_pos(float pos);

  void update_geometry(const sf::Vector2i& mouse_pos);

  sf::RectangleShape m_container;
  sf::RectangleShape m_thumb;
  ScrollState m_state{};
};

class ScrollBarContainer {
 private:
  float m_content_height{0};
  float m_viewport_height{0};
  float m_viewport_width{0};
  std::vector<std::function<void(const ScrollDimensions&)>> m_layout_observers;

 protected:
  void subscribe_to_layout(
      std::function<void(const ScrollDimensions&)> callback) {
    m_layout_observers.push_back(std::move(callback));
    notify();
  }

  void set_view_height(float viewport_height) {
    m_viewport_height = viewport_height;
    notify();
  }

  void set_content_height(float content_height) {
    m_content_height = content_height;
    notify();
  }

  void set_view_width(float viewport_width) {
    m_viewport_width = viewport_width;
    notify();
  }

 private:
  void notify() {
    ScrollDimensions dims{.view_height = m_viewport_height,
                          .content_height = m_content_height,
                          .view_width = m_viewport_width};
    for (const auto& callback : m_layout_observers) {
      callback(dims);
    }
  }
};

}  // namespace my_b::ui
