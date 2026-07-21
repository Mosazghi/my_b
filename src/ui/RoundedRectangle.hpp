#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Shape.hpp>

namespace my_b::ui {
class RoundedRectangleShape : public sf::Shape {
 public:
  RoundedRectangleShape(sf::Vector2f size, float radius,
                        std::size_t cornerPointsCount = 10);

  void set_size(sf::Vector2f size);
  void set_corner_radius(float radius);
  std::size_t getPointCount() const override;
  sf::Vector2f getPoint(std::size_t index) const override;

 private:
  sf::Vector2f m_size;
  float m_radius;
  std::size_t m_cornerPointCount;
  float m_anglePerPoint;
};
}  // namespace my_b::ui
