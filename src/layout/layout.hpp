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
class Layout {
 public:
  explicit Layout(const std::vector<Token>& tokens);

  std::vector<PositionTextPair> layout(const std::vector<Token>& tokens,
                                       sf::Font& font, int window_width);

 private:
  void token(const auto& token);
  int m_cursor_x = HSTEP;
  int m_cursor_y = VSTEP;
  std::string m_style{"roman"};
  std::string m_weight{"normal"};
};
};  // namespace layout
