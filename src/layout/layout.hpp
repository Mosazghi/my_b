#pragma once

#include <SFML/Window/Event.hpp>
#include <cstdint>
#include <optional>
#include <string>
#include <tuple>
#include <variant>
#include <vector>
#include "SFML/Graphics/Text.hpp"
#include "SFML/System/String.hpp"
namespace layout {

inline constexpr auto HSTEP{13};
inline constexpr auto VSTEP{15};

namespace TextSize {
enum : std::uint8_t {
  Large = 16,
  Medium = 14,
  Normal = 11,
  Small = 9,
  Super = Medium / 2
};
};

enum class LayoutElementType : std::uint8_t { Emoji, Text };
enum class VerticalAlign : std::uint8_t { Baseline, Super, Sub };

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
  VerticalAlign vertical_align{VerticalAlign::Baseline};
};

using Token = std::variant<Text, Tag>;
using X = double;
using Y = double;
using PositionTextPair = std::tuple<X, Y, LayoutElement, sf::Text>;
using LineElement = std::tuple<X, LayoutElement, sf::Text>;

struct LayoutContext {
  int size{TextSize::Normal};
  int window_width{};
  double cursor_x{HSTEP};
  double cursor_y{VSTEP};
  std::string style{"roman"};
  std::string weight{"normal"};
  const Tag* current_tag{nullptr};
  VerticalAlign vertical_align{VerticalAlign::Baseline};
  sf::Font& font;
  std::vector<PositionTextPair> display_content{};
  std::vector<LineElement> line{};
};

std::vector<PositionTextPair> compute(const std::vector<Token>& tokens,
                                      sf::Font& font, int window_width);
};  // namespace layout
