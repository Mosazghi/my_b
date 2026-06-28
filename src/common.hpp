#pragma once
#include <SFML/Config.hpp>
#include <SFML/System/String.hpp>
#include <string>
#include <vector>
#include "layout/layout.hpp"

namespace common {

std::vector<layout::Token> lex(std::string& body);
bool isEmoji(sf::Uint32 codepoint);
std::string get_emoji_id(sf::Uint32 codepoint);

}  // namespace common
