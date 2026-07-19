#pragma once

namespace my_b::ui {
class Element {
 public:
  virtual ~Element() = default;
  virtual void draw() = 0;
};

}  // namespace my_b::ui
