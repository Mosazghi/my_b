#include "ResourceManager.h"
#include <format>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <system_error>
#include <tuple>
#include "SFML/Graphics/Texture.hpp"
#include "layout/layout.hpp"

namespace resource {

std::unordered_map<std::string, sf::Texture> ResourceManager::m_texture_cache;
std::map<std::tuple<std::string, int, std::string, std::string>,
         std::tuple<sf::Text, sf::String>>
    ResourceManager::m_font_cache;

std::optional<sf::Texture> ResourceManager::get_texture(const std::string& id) {
  auto exists = m_texture_cache.contains(id);
  if (exists) {
    return m_texture_cache.at(id);
  }

  std::string texture_path = std::format("assets/emojis/{}.PNG", id);
  sf::Texture texture{};
  if (!texture.loadFromFile(texture_path)) {
    return {};
  }
  m_texture_cache[id] = texture;
  return texture;
}

std::tuple<sf::Text, sf::String> ResourceManager::get_font(
    const std::string& word, const layout::LayoutContext& ctx) {
  auto key = std::make_tuple(word, ctx.size, ctx.weight, ctx.style);
  if (!m_font_cache.contains(key)) {
    sf::String sf_word = sf::String::fromUtf8(word.begin(), word.end());

    sf::Text sf_text(sf_word, ctx.font, ctx.size);
    sf::Uint32 style = sf::Text::Regular;
    if (ctx.weight == "bold") {
      style |= sf::Text::Bold;
    }
    if (ctx.style == "italic") {
      style |= sf::Text::Italic;
    }

    sf_text.setFillColor(sf::Color::Black);
    const auto bounds = sf_text.getLocalBounds();
    sf_text.setOrigin(bounds.left, bounds.top);
    sf_text.setStyle(style);

    m_font_cache[key] = std::make_tuple(sf_text, sf_word);
  }

  return m_font_cache.at(key);
}

}  // namespace resource
