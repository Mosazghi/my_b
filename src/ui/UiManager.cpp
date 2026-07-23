#include "UiManager.hpp"
#include <fmt/base.h>
namespace my_b::ui {

UiManager::UiManager(sf::RenderWindow& window) : m_window(window) {}

UiManager::~UiManager() = default;

void UiManager::remove_element(UiElement* element) {
  auto it = std::ranges::find_if(
      m_ui_elements, [element](const std::unique_ptr<UiElement>& e) {
        return e.get() == element;
      });
  if (it != m_ui_elements.end()) {
    m_ui_elements.erase(it);
  }
}

void UiManager::draw(int) {
  for (const auto& element : m_ui_elements) {
    m_window.draw(*element);
  }
}

void UiManager::handle_event(sf::Event& event) {
  for (const auto& element : m_ui_elements) {
    element->handle_event(event, m_window);
  }
}

}  // namespace my_b::ui
