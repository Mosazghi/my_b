#include "common.hpp"
#include <openssl/evp.h>
#include <unicode/uchar.h>
#include <unicode/urename.h>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <variant>
#include <vector>
#include "SFML/Graphics/Font.hpp"
#include "const.hpp"

namespace common {

std::vector<Token> lex(std::string& body) {
  bool in_tag{};
  std::string buffer{};
  std::vector<Token> result{};

  body = std::regex_replace(body, std::regex("&lt;"), "<");
  body = std::regex_replace(body, std::regex("&gt;"), ">");

  for (const auto& c : body) {
    if (c == '<') {
      in_tag = true;
      if (!buffer.empty()) {
        result.emplace_back(Text(buffer));
      }
      buffer.clear();
    } else if (c == '>') {
      in_tag = false;
      result.emplace_back(Tag(buffer));
      buffer.clear();
    } else {
      buffer += c;
    }
  }
  if (!in_tag and !buffer.empty()) {
    result.emplace_back(Text(buffer));
  }
  return result;
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

std::vector<PositionTextPair> layout(const std::vector<Token>& tokens,
                                     sf::Font& font, int window_width) {
  std::vector<PositionTextPair> display_list;

  auto cursor_x = consts::HSTEP;
  auto cursor_y = consts::VSTEP;

  const float line_space = font.getLineSpacing(16) * 1.25f;
  const float space_width = font.getGlyph(' ', 16, false).advance;

  std::string style{"roman"};
  std::string weight{"normal"};

  for (const auto& tok : tokens) {
    if (std::holds_alternative<Text>(tok)) {
      const auto& text = std::get<Text>(tok);
      std::istringstream stream(text.text);
      std::string word;

      while (stream >> word) {
        sf::String sf_word = sf::String::fromUtf8(word.begin(), word.end());
        sf::Text wrd(sf_word, font, 16);

        sf::Uint32 sfml_style = sf::Text::Regular;

        if (weight == "bold") {
          sfml_style |= sf::Text::Bold;
        }
        if (style == "italic") {
          sfml_style |= sf::Text::Italic;
        }
        wrd.setStyle(sfml_style);
        const auto word_width = wrd.getLocalBounds().width;

        LayoutElement element{.type = LayoutElementType::TEXT,
                              .value = sf_word};

        if (!sf_word.isEmpty() && isEmoji(sf_word[0])) {
          element.type = LayoutElementType::EMOJI;
        }
        if (cursor_x + word_width > window_width - consts::HSTEP) {
          cursor_y += line_space;
          cursor_x = consts::HSTEP;
        }

        display_list.emplace_back(cursor_x, cursor_y, element, wrd);
        cursor_x += word_width + space_width;
      }
    } else {
      const auto& tag = std::get<Tag>(tok).tag;
      if (tag == "i") {
        style = "italic";
      } else if (tag == "/i") {
        style = "roman";
      } else if (tag == "b") {
        weight = "bold";
      } else if (tag == "/b") {
        weight = "normal";
      }
    }
  }

  return display_list;
}

}  // namespace common
