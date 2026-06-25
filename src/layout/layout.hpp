#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include "SFML/Graphics/Text.hpp"
#include "SFML/System/String.hpp"
namespace layout {

inline constexpr auto HSTEP = 13;
inline constexpr auto VSTEP = 15;

enum class LayoutElementType : std::uint8_t { EMOJI, TEXT };

struct LayoutElement {
  LayoutElementType type;
  sf::String value{};
};

struct Text {
  std::string text;
  explicit Text(std::string t) : text(std::move(t)) {}
};

struct Tag {
  std::string tag;
  explicit Tag(std::string t) : tag(std::move(t)) {}
};

using Token = std::variant<Text, Tag>;

using Token = std::variant<Text, Tag>;
using PositionTextPair = std::tuple<int, int, LayoutElement, sf::Text>;

struct LayoutContext {
  float cursor_x = HSTEP;
  float cursor_y = VSTEP;
  std::string style{"roman"};
  std::string weight{"normal"};
  int size{12};
  std::vector<PositionTextPair> display_list;
};

std::vector<PositionTextPair> compute(const std::vector<Token>& tokens,
                                      sf::Font& font, int window_width);
};  // namespace layout
