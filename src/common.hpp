#pragma once
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

using PositionTextPair = std::tuple<int, int, char>;

inline std::vector<PositionTextPair> layout(std::string& text) {
  std::vector<PositionTextPair> display_list(text.length());

  auto cursor_x = consts::HSTEP;
  auto cursor_y = consts::VSTEP;

  for (const auto& c : text) {
    display_list.emplace_back(cursor_x, cursor_y, c);

    if (c == U'\n') {
      cursor_x = consts::HSTEP;
      cursor_y += consts::VSTEP;
      continue;
    }
    if (cursor_x >= consts::WINDOW_WIDTH - consts::HSTEP) {
      cursor_x = consts::HSTEP;
      cursor_y += consts::VSTEP;
    }

    cursor_x += consts::HSTEP;
  }

  return display_list;
}

}  // namespace common
