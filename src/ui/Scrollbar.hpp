#pragma once
#include "Element.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Window/Event.hpp"
namespace ui {

struct ScrollState {
  int content_height{};
  int viewport_height{};
  int scroll_pos{};
};
enum class ScrollDirection { UP, DOWN };

class ScrollBar : public Element {
 public:
  explicit ScrollBar(sf::RenderWindow& window);
  ~ScrollBar() override = default;

  void update(int content_height, int viewport_height);
  void draw() override;
  void handleEvent(const sf::Event& event) override;
  int get_scroll_pos() const { return m_state.scroll_pos; }

  void mouse_click_scroll(const sf::Event& e);
  void mouse_hold_scroll(const sf::Event& e);
  void mouse_scroll(const sf::Event& e);
  void scrolldown(ScrollDirection direction, const int scroll_step = 100);

 private:
  sf::RectangleShape m_container;
  sf::RectangleShape m_thumb;
  sf::RenderWindow& m_window;
  ScrollState m_state{};
};

}  // namespace ui
