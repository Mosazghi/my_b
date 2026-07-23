#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <memory>
#include <vector>
#include "UiElement.hpp"
namespace my_b::ui {
class UiManager {
 public:
  UiManager(sf::RenderWindow& window);
  ~UiManager();
  void draw(int y_offset = 0);
  void update(sf::Event& event);
  void remove_element(UiElement* element);

  template <typename T, typename... Args>
  T* create_element(Args&&... args) {
    auto element = std::make_unique<T>(std::forward<Args>(args)...);
    T* ptr = element.get();
    m_ui_elements.push_back(std::move(element));
    return ptr;
  }

 private:
  sf::RenderWindow& m_window;
  std::vector<std::unique_ptr<UiElement>> m_ui_elements;
};

}  // namespace my_b::ui
