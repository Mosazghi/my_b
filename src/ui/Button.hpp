#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdint>
#include <functional>
#include "RoundedRectangle.hpp"
#include "UiElement.hpp"

namespace my_b::ui {

class Button : public UiElement {
 public:
  enum class State : std::uint8_t { Normal, Hover, Pressed };
  Button(const sf::Vector2f& position, const sf::Vector2f& size,
         const std::string& text, const sf::Font& font);
  ~Button() override = default;

  void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
  void handle_event(const sf::Event& event, sf::RenderWindow& window) override;
  std::string get_name() const override { return "Button"; }
  void set_text(const std::string& text);
  void set_on_click(std::function<void()> callback);
  void set_position(const sf::Vector2f& position);
  void set_size(const sf::Vector2f& size);
  void set_text_color(const sf::Color& color);
  void set_font(const sf::Font& font);
  void set_text_size(unsigned int size);
  void set_normal_color(const sf::Color& color);
  void set_hover_color(const sf::Color& color);
  void set_pressed_color(const sf::Color& color);
  void set_scroll_offset(float offset) override;

 private:
  void update_text();
  void update_visuals();
  sf::Color m_normalColor;
  sf::Color m_hoverColor;
  sf::Color m_pressedColor;
  std::string m_text;
  std::function<void()> m_on_click;
  RoundedRectangleShape m_rect;
  sf::Vector2f m_anchor{};
  State m_state = State::Normal;
  sf::Text m_text_obj;
};

}  // namespace my_b::ui
