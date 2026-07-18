#pragma once
#include <SFML/Graphics/Text.hpp>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include "layout/layout.hpp"

namespace resource {
class ResourceManager {
 public:
  static std::optional<sf::Texture> get_texture(const std::string& id);
  static std::tuple<sf::Text, sf::String> get_font(
      const std::string& text, const layout::LayoutContext& ctx);
  static std::unordered_map<std::string, sf::Texture> m_texture_cache;
  static std::map<std::tuple<std::string, int, std::string, std::string>,
                  std::tuple<sf::Text, sf::String>>
      m_font_cache;
};
}  // namespace resource
