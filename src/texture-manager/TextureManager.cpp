#include "TextureManager.h"
#include <format>
#include "SFML/Graphics/Texture.hpp"

namespace texture {

std::unordered_map<std::string, sf::Texture> TextureManager::m_texture_cache;

sf::Texture TextureManager::get(const std::string& id) {
  auto exists = m_texture_cache.contains(id);
  if (exists) {
    return m_texture_cache.at(id);
  }

  std::string texture_path = std::format("assets/emojis/{}.PNG", id);
  sf::Texture texture{};
  texture.loadFromFile(texture_path);
  m_texture_cache[id] = texture;
  return texture;
}

}  // namespace texture
