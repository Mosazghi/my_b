#include "common.hpp"
#include <unicode/uchar.h>
#include <unicode/urename.h>
#include <SFML/System/String.hpp>
#include <iomanip>
#include <regex>
#include "const.hpp"

namespace common {

std::string lex(std::string& body) {
  bool in_tag{};
  std::string text{};

  body = std::regex_replace(body, std::regex("&lt;"), "<");
  body = std::regex_replace(body, std::regex("&gt;"), ">");

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

bool isEmoji(sf::Uint32 codepoint) {
  auto c = static_cast<UChar32>(codepoint);
  return u_hasBinaryProperty(c, UCHAR_EMOJI_PRESENTATION);
}

std::string get_emoji_id(sf::Uint32 codepoint) {
  std::stringstream id_stream;
  id_stream << std::hex << std::uppercase << std::setfill('0') << std::setw(5)
            << static_cast<uint32_t>(codepoint) << std::dec;
  return id_stream.str();
}

std::vector<PositionTextPair> layout(const std::string& text,
                                     int window_width) {
  std::vector<PositionTextPair> display_list;
  if (text.empty()) {
    return display_list;
  }
  display_list.reserve(text.size());
  sf::String decoded = sf::String::fromUtf8(text.begin(), text.end());

  auto cursor_x = consts::HSTEP;
  auto cursor_y = consts::VSTEP;

  for (const std::uint32_t c : decoded) {
    DecodedType type{.type = TextureType::TEXT, .value = c};
    if (isEmoji(c)) {
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
