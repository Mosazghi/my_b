#pragma once
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Window/Event.hpp>

namespace my_b::ui {
class UiElement : public sf::Drawable, public sf::Transformable {
 public:
  virtual ~UiElement() = default;
  virtual void handle_event(const sf::Event& event,
                            sf::RenderWindow& window) = 0;
  virtual std::string get_name() const = 0;
  virtual void set_scroll_offset(float /*offset */) {}
};

}  // namespace my_b::ui
