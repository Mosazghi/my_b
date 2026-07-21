#pragma once
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Vector2.hpp>
#include <chrono>
#include <cstdint>
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Window/Event.hpp"
#include "UiElement.hpp"

namespace my_b::ui {

struct ScrollState {
  int content_height{};
  int viewport_height{};
  int scroll_pos{};
  bool is_hovering_thumb{false};
  bool is_hovering_container{false};
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

  std::string get_name() const override { return "ScrollBar"; }
  void set_heights(int content_height, int viewport_height);

  [[nodiscard]] int get_current_roll_pos() const { return m_state.scroll_pos; }
  [[nodiscard]] float get_width() const { return SCROLL_BAR_WIDTH; }

 private:
  void mouse_click_scroll(const sf::Event& e, const sf::Vector2i& mouse_pos);
  void mouse_hold_scroll(const sf::Event& e, const sf::Vector2i& mouse_pos);
  void mouse_scroll(const sf::Event& e);
  void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
  [[nodiscard]] float get_scroll_pos_from_mouse(
      const sf::Vector2f& mouse_pos) const;
  void set_scroll_pos(float pos);

  void update_geometry(const sf::Vector2i& mouse_pos,
                       const sf::Vector2u& windowSize);

  sf::RectangleShape m_container;
  sf::RectangleShape m_thumb;
  ScrollState m_state{};
};

}  // namespace my_b::ui
