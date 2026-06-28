#pragma once

#include <cstdint>
#include <string>
#include <tuple>
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
using PositionTextPair = std::tuple<double, double, LayoutElement, sf::Text>;
using Line = std::tuple<double, LayoutElement, sf::Text>;

struct LayoutContext {
  int size{12};
  int window_width{};
  double cursor_x{HSTEP};
  double cursor_y{VSTEP};
  std::string style{"roman"};
  std::string weight{"normal"};
  std::vector<PositionTextPair> display_list{};
  std::vector<Line> line{};
  sf::Font& font;
};

std::vector<PositionTextPair> compute(const std::vector<Token>& tokens,
                                      sf::Font& font, int window_width);
};  // namespace layout
