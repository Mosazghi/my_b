#pragma once

#include <cstdint>
#include <optional>
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

struct Text {
  std::string text{};
  explicit Text(std::string t) : text(std::move(t)) {}
};

struct Tag {
  std::string tag{};
  std::string rest{};
  explicit Tag(std::string t, std::string r)
      : tag(std::move(t)), rest(std::move(r)) {}
};
struct LayoutElement {
  LayoutElementType type{};
  sf::String value{};
  std::optional<Tag> tag{};
};

using Token = std::variant<Text, Tag>;
using X = double;
using Y = double;
using PositionTextPair = std::tuple<X, Y, LayoutElement, sf::Text>;
using LineElement = std::tuple<X, LayoutElement, sf::Text>;

struct LayoutContext {
  int size{16};
  int window_width{};
  double cursor_x{HSTEP};
  double cursor_y{VSTEP};
  std::string style{"roman"};
  std::string weight{"normal"};
  const Tag* current_tag{nullptr};
  std::vector<PositionTextPair> display_content{};
  std::vector<LineElement> line{};
  sf::Font& font;
};

std::vector<PositionTextPair> compute(const std::vector<Token>& tokens,
                                      sf::Font& font, int window_width);
};  // namespace layout
