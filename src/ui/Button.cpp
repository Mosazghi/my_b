#include "Button.hpp"
#include <fmt/base.h>
#include <SFML/Window/Cursor.hpp>
namespace my_b::ui {

Button::Button(const sf::Vector2f& position, const sf::Vector2f& size,
               const std::string& text, const sf::Font& font)
    : m_normalColor(sf::Color(70, 70, 70)),
      m_hoverColor(sf::Color(100, 100, 100)),
      m_pressedColor(sf::Color(40, 40, 40)),
      m_rect(size, 10.0f) {
  m_rect.setFillColor(m_normalColor);
  m_rect.setPosition(position);
  m_text_obj.setFont(font);
  m_text_obj.setString(text);
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const {
  states.transform *= getTransform();
  target.draw(m_rect, states);
  target.draw(m_text_obj, states);
}

void Button::handle_event(const sf::Event& event, sf::RenderWindow& window) {
  static bool was_hovered = false;
  const auto mouse_pos = sf::Mouse::getPosition(window);
  const auto is_hovered =
      m_rect.getGlobalBounds().contains(static_cast<sf::Vector2f>(mouse_pos));

  if (event.type == sf::Event::MouseMoved) {
    if (m_state != State::Pressed) {
      m_state = is_hovered ? State::Hover : State::Normal;
    }
  }

  if (event.type == sf::Event::MouseButtonPressed) {
    if (is_hovered) {
      m_state = State::Pressed;
    }
  }

  if (event.type == sf::Event::MouseButtonReleased &&
      event.mouseButton.button == sf::Mouse::Left) {
    if (m_state == State::Pressed) {
      m_state = is_hovered ? State::Hover : State::Normal;
      if (is_hovered && m_on_click) {
        m_on_click();
      }
    }
  }
  sf::Cursor cursor;

  if (cursor.loadFromSystem(sf::Cursor::Hand) && is_hovered) {
    window.setMouseCursor(cursor);
  } else {
    if (cursor.loadFromSystem(sf::Cursor::Arrow) && was_hovered) {
      window.setMouseCursor(cursor);
    }
  }
  was_hovered = is_hovered;
  update_visuals();
}

void Button::set_on_click(std::function<void()> on_click) {
  m_on_click = std::move(on_click);
}

void Button::set_text(const std::string& text) {
  m_text_obj.setString(text);
  update_text();
}

void Button::set_position(const sf::Vector2f& position) {
  m_rect.setPosition(position);
  update_text();
}

void Button::set_size(const sf::Vector2f& size) {
  m_rect.set_size(size);
  update_text();
}

void Button::set_text_color(const sf::Color& color) {
  m_text_obj.setFillColor(color);
  update_text();
}

void Button::set_font(const sf::Font& font) {
  m_text_obj.setFont(font);
  update_text();
}

void Button::set_text_size(unsigned int size) {
  m_text_obj.setCharacterSize(size);
  update_text();
}

void Button::update_text() {
  const sf::FloatRect textRect = m_text_obj.getLocalBounds();
  m_text_obj.setOrigin(textRect.left + textRect.width / 2.0f,
                       textRect.top + textRect.height / 2.0f);

  const sf::FloatRect buttonRect = m_rect.getGlobalBounds();
  m_text_obj.setPosition(buttonRect.left + buttonRect.width / 2.0f,
                         buttonRect.top + buttonRect.height / 2.0f);
}

void Button::update_visuals() {
  switch (m_state) {
    case State::Normal:
      m_rect.setFillColor(m_normalColor);
      break;
    case State::Hover:
      m_rect.setFillColor(m_hoverColor);
      break;
    case State::Pressed:
      m_rect.setFillColor(m_pressedColor);
      break;
  }
}

void Button::set_normal_color(const sf::Color& color) {
  m_normalColor = color;
  update_visuals();
}

void Button::set_hover_color(const sf::Color& color) {
  m_hoverColor = color;
  update_visuals();
}

void Button::set_pressed_color(const sf::Color& color) {
  m_pressedColor = color;
  update_visuals();
}

}  // namespace my_b::ui
