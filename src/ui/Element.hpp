#pragma once

#include "SFML/Window/Event.hpp"
namespace ui {
class Element {
 public:
  virtual ~Element() = default;
  virtual void draw() = 0;
  virtual void handleEvent(const sf::Event& event) = 0;
};

}  // namespace ui
