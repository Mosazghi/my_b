#pragma once

namespace ui {
class Element {
 public:
  virtual ~Element() = default;
  virtual void draw() = 0;
};

}  // namespace ui
