#pragma once
#include <unicode/uchar.h>  // Main ICU header for character properties
#include <unicode/urename.h>
#include <SFML/System/String.hpp>
#include <cstdint>
#include <regex>
#include <string>
#include "SFML/Config.hpp"
#include "const.hpp"
namespace common {

inline std::string lex(std::string& body) {
  bool in_tag{};
  std::string text{};

  body = std::regex_replace(body, std::regex("&lt;"), "<");
  body = std::regex_replace(body, std::regex("&gt;"), ">");

  // if (is_scheme_in(Scheme::VIEW_SOURCE)) {
  //   std::cout << body;
  //   return "";
  // }

  for (const auto& c : body) {
    if (c == '<') {
      in_tag = true;
    } else if (c == '>') {
      in_tag = false;
    } else if (!in_tag) {
      text += c;
    }
  }
  return text;
}

enum class TextureType : std::uint8_t { EMOJI, TEXT };
struct DecodedType {
  TextureType type;
  sf::Uint32 value{};
};
using PositionTextPair = std::tuple<int, int, DecodedType>;

inline bool isEmoji(sf::Uint32 codepoint) {
  auto c = static_cast<UChar32>(codepoint);
  return u_hasBinaryProperty(c, UCHAR_EMOJI_PRESENTATION);
}

inline std::vector<PositionTextPair> layout(const std::string& text,
                                            int window_width) {
  std::vector<PositionTextPair> display_list;
  if (text.empty()) {
    return display_list;
  }
  display_list.reserve(text.size());
  sf::String decoded = sf::String::fromUtf8(text.begin(), text.end());

  auto cursor_x = consts::HSTEP;
  auto cursor_y = consts::VSTEP;

  for (const sf::Uint32 c : decoded) {
    DecodedType type{.type = TextureType::TEXT, .value = c};
    if (isEmoji(c)) {
      sf::String emojiStr(c);
      std::string utf8Emoji = emojiStr.toAnsiString();
      type.type = TextureType::EMOJI;
    }
    display_list.emplace_back(cursor_x, cursor_y, type);

    if (c == U'\n') {
      cursor_x = consts::HSTEP;
      cursor_y += consts::VSTEP;
      continue;
    }
    if (cursor_x >= window_width - consts::HSTEP) {
      cursor_x = consts::HSTEP;
      cursor_y += consts::VSTEP;
    }

    cursor_x += consts::HSTEP;
  }

  return display_list;
}

}  // namespace common
