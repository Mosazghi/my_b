#pragma once
#include <SFML/Config.hpp>
#include <SFML/System/String.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include "layout/layout.hpp"

namespace my_b::common {

std::vector<layout::Token> lex(std::string& body);
bool isEmoji(std::uint32_t codepoint);
std::string get_emoji_id(std::uint32_t codepoint);

}  // namespace my_b::common
