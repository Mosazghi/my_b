#include "UiManager.hpp"
#include <fmt/base.h>
#include "Scrollbar.hpp"
namespace my_b::ui {

UiManager::UiManager(sf::RenderWindow& window) : m_window(window) {}

UiManager::~UiManager() = default;

void UiManager::removeElement(UiElement* element) {
  auto it = std::ranges::find_if(
      m_ui_elements, [element](const std::unique_ptr<UiElement>& e) {
        return e.get() == element;
      });
  if (it != m_ui_elements.end()) {
    m_ui_elements.erase(it);
  }
}

void UiManager::draw(int y_offset) {
  for (const auto& element : m_ui_elements) {
    element->set_scroll_offset(static_cast<float>(y_offset));
    m_window.draw(*element);
  }
  m_applied_y_offset = static_cast<float>(y_offset);
}

void UiManager::update(sf::Event& event) {
  for (const auto& element : m_ui_elements) {
    element->handle_event(event, m_window);
  }
}

}  // namespace my_b::ui
