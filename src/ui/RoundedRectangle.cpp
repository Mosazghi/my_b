#include "RoundedRectangle.hpp"
#include <SFML/System/Vector2.hpp>
#include <cmath>

namespace my_b::ui {
RoundedRectangleShape::RoundedRectangleShape(const sf::Vector2f size,
                                             const float radius,
                                             const std::size_t cornerPointCount)
    : m_size(size), m_radius(radius), m_cornerPointCount(cornerPointCount) {
  update();
}

void RoundedRectangleShape::set_size(const sf::Vector2f size) {
  m_size = size;
  update();
}

void RoundedRectangleShape::set_corner_radius(const float radius) {
  m_radius = radius;
  update();
}

std::size_t RoundedRectangleShape::getPointCount() const {
  return m_cornerPointCount * 4;
}

sf::Vector2f RoundedRectangleShape::getPoint(std::size_t index) const {
  if (m_radius <= 0 || m_cornerPointCount == 1) {
    sf::RectangleShape rect(m_size);
    return rect.getPoint(index);
  }

  // Determine which corner we are building (0=Top-Right, 1=Bottom-Right,
  // 2=Bottom-Left, 3=Top-Left)
  std::size_t corner = index / m_cornerPointCount;
  float angle = 0;
  bool isLastPointOnCorner =
      (index % m_cornerPointCount == m_cornerPointCount - 1);
  if (isLastPointOnCorner) {
    angle = 90.0f;
  } else {
    angle = (index % m_cornerPointCount) * m_anglePerPoint;
  }

  sf::Vector2f point{};

  switch (corner) {
    case 0:
      point = {m_radius, m_radius};
      angle -= 180;
      break;
    case 1:
      point = {m_size.x - m_radius, m_radius};
      angle -= 90;
      break;
    case 2:
      point = {m_size.x - m_radius, m_size.y - m_radius};
      break;
    default:
      point = {m_radius, m_size.y - m_radius};
      angle += 90;
  }
  point += {cosf(angle * M_PI / 180) * m_radius,
            sinf(angle * M_PI / 180) * m_radius};
  return point;
}

}  // namespace my_b::ui
