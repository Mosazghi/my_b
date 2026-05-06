#pragma once
#include <SFML/System/String.hpp>
#include <iostream>
#include <regex>
#include <string>
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

using PositionTextPair = std::tuple<int, int, sf::Uint32>;

inline std::vector<PositionTextPair> layout(const std::string& text,
                                            int window_width) {
  std::vector<PositionTextPair> display_list;
  display_list.reserve(text.size());
  auto decoded = sf::String::fromUtf8(text.begin(), text.end());

  auto cursor_x = consts::HSTEP;
  auto cursor_y = consts::VSTEP;

  for (const auto c : decoded) {
    display_list.emplace_back(cursor_x, cursor_y, c);

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
