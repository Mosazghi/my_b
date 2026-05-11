#pragma once
#include <string>
#include <unordered_map>
#include "SFML/Graphics/Texture.hpp"

namespace texture {
class TextureManager {
 public:
  static sf::Texture get(const std::string& id);

 private:
  static std::unordered_map<std::string, sf::Texture> m_texture_cache;
};
}  // namespace texture
