#pragma once
#include <chrono>
#include <cstdint>
#include "Element.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Window/Event.hpp"
namespace ui {

struct ScrollState {
  int content_height{};
  int viewport_height{};
  int scroll_pos{};
  std::chrono::steady_clock::time_point last_scroll_time =
      std::chrono::steady_clock::now();
};

enum class ScrollDirection : std::uint8_t { UP, DOWN };

class ScrollBar : public Element {
  static constexpr float SCROLL_BAR_WIDTH{20.f};

 public:
  explicit ScrollBar(sf::RenderWindow& window);
  ~ScrollBar() override = default;

  void update(int content_height, int viewport_height);
  void draw() override;
  int get_current_roll_pos() const { return m_state.scroll_pos; }
  float get_width() const { return SCROLL_BAR_WIDTH; }

  void mouse_click_scroll(const sf::Event& e);
  void mouse_hold_scroll(const sf::Event& e);
  void mouse_scroll(const sf::Event& e);

 private:
  float get_scroll_pos_from_mouse(const sf::Vector2i& mouse_pos) const;
  void set_scroll_pos(float pos);
  sf::RectangleShape m_container;
  sf::RectangleShape m_thumb;
  sf::RenderWindow& m_window;
  ScrollState m_state{};
};

}  // namespace ui
