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
#include <vector>

namespace common {

std::vector<layout::Token> lex(std::string& body) {
  bool in_tag{};
  std::string buffer{};
  std::vector<layout::Token> result{};

  using namespace layout;

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

}  // namespace common
