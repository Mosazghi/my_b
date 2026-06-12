#include "common.hpp"
#include <openssl/evp.h>
#include <unicode/uchar.h>
#include <unicode/urename.h>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <regex>
#include <sstream>
#include "SFML/Graphics/Font.hpp"
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
                                     const sf::Font& font, int window_width) {
  std::vector<PositionTextPair> display_list;
  if (text.empty()) {
    return display_list;
  }

  auto cursor_x = consts::HSTEP;
  auto cursor_y = consts::VSTEP;

  const float line_space = font.getLineSpacing(16) * 1.25f;
  const float space_width = font.getGlyph(U' ', 16, false).advance;

  std::istringstream stream(text);
  std::string word;

  while (stream >> word) {
    sf::String sf_word = sf::String::fromUtf8(word.begin(), word.end());
    sf::Text wrd(sf_word, font, 16);

    const auto word_width = wrd.getLocalBounds().width;

    DecodedElement element{.type = TextureType::TEXT, .value = sf_word};

    if (!sf_word.isEmpty() && isEmoji(sf_word[0])) {
      element.type = TextureType::EMOJI;
    }
    // if (cursor_x + word_width > window_width - consts::HSTEP) {
    //   std::cout << "newline\n";
    //   cursor_y += line_space;
    //   cursor_x = consts::HSTEP;
    // }

    display_list.emplace_back(cursor_x, cursor_y, element);
    cursor_x += word_width + space_width;

    if (sf_word[0] == '\n') {
      std::cout << "NEWLINE\n";
      cursor_x = consts::HSTEP;
      cursor_y += line_space;
      continue;
    }

    if (cursor_x + word_width > window_width - consts::HSTEP) {
      cursor_y += line_space;
      cursor_x = consts::HSTEP;
    }
  }

  return display_list;
}

}  // namespace common
