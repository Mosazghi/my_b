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
#include "utils.hpp"

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
        utils::trim(buffer);
        result.emplace_back(Text(buffer));
      }
      buffer.clear();
    } else if (c == '>') {
      in_tag = false;
      std::string rest = "";
      const auto first_space = buffer.find(' ');
      if (first_space != std::string::npos) {
        rest = buffer.substr(first_space, buffer.size() - 1);
      }
      if (first_space != std::string::npos) {
        buffer = buffer.substr(0, first_space);
      } else {
        buffer = std::regex_replace(buffer, std::regex("\\s+"), "");
      }

      result.emplace_back(Tag(buffer, rest));
      buffer.clear();
    } else {
      buffer += c;
    }
  }
  if (!in_tag and !buffer.empty()) {
    utils::trim(buffer);
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
