#pragma once

#include <SFML/Window/Event.hpp>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/System/String.hpp"
namespace my_b::layout {

inline constexpr auto HSTEP{13};
inline constexpr auto VSTEP{15};

namespace TextSize {
enum : std::uint8_t {
  Large = 18,
  Medium = Large - 2,
  Normal = Medium - 2,
  Small = Normal - 6,
  Super = Medium / 2,
  Sub = Super
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
  std::string parent_tag{};
  explicit Tag(std::string t, std::string r, std::string p)
      : tag(std::move(t)), rest(std::move(r)), parent_tag(std::move(p)) {}
  bool is_closing() const { return !tag.empty() && tag[0] == '/'; }
};
struct LayoutElement {
  LayoutElementType type{};
  VerticalAlign vertical_align{VerticalAlign::Baseline};
  sf::String value{};
  std::optional<Tag> tag{};
};

using Token = std::variant<Text, Tag>;
using X_POS = double;
using Y_POS = double;
using PositionTextPair = std::tuple<X_POS, Y_POS, LayoutElement, sf::Text>;
using LineElement = std::tuple<X_POS, LayoutElement, sf::Text>;

struct LayoutContext {
  int size{TextSize::Normal};
  int window_width{};
  double cursor_x{HSTEP};
  double cursor_y{VSTEP};
  std::string style{"roman"};
  std::string weight{"normal"};
  std::optional<Tag> current_tag{std::nullopt};
  VerticalAlign vertical_align{VerticalAlign::Baseline};
  sf::Font* font;
  sf::Font* default_font;
  std::vector<PositionTextPair> display_content{};
  std::vector<LineElement> line{};

  bool has_tag(std::string_view tag) const {
    return current_tag &&
           (current_tag->tag == tag || current_tag->parent_tag == tag);
  }
};

std::vector<PositionTextPair> compute(const std::vector<Token>& tokens,
                                      sf::Font* default_font, int window_width);
};  // namespace my_b::layout
