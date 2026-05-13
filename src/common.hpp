#pragma once
#include <SFML/Config.hpp>
#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

namespace common {

enum class TextureType : std::uint8_t { EMOJI, TEXT };

struct DecodedType {
  TextureType type;
  sf::Uint32 value{};
};

using PositionTextPair = std::tuple<int, int, DecodedType>;

std::string lex(std::string& body);
bool isEmoji(sf::Uint32 codepoint);
std::string get_emoji_id(sf::Uint32 codepoint);
std::vector<PositionTextPair> layout(const std::string& text, int window_width);

}  // namespace common
