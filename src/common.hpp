#pragma once
#include <SFML/Config.hpp>
#include <SFML/System/String.hpp>
#include <cstdint>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>
#include "SFML/Graphics/Font.hpp"

namespace common {

enum class LayoutElementType : std::uint8_t { EMOJI, TEXT };

struct LayoutElement {
  LayoutElementType type;
  sf::String value{};
};

using PositionTextPair = std::tuple<int, int, LayoutElement, sf::Text>;

struct Text {
  std::string text;
  explicit Text(std::string t) : text(std::move(t)) {}
};

struct Tag {
  std::string tag;
  explicit Tag(std::string t) : tag(std::move(t)) {}
};

using Token = std::variant<Text, Tag>;

std::vector<Token> lex(std::string& body);
bool isEmoji(sf::Uint32 codepoint);
std::string get_emoji_id(sf::Uint32 codepoint);

std::vector<PositionTextPair> layout(const std::vector<Token>& tokens,
                                     sf::Font& font, int window_width);

}  // namespace common
