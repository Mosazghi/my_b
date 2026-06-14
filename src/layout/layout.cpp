#include "layout.hpp"
#include <sstream>
#include "common.hpp"

namespace layout {

std::vector<PositionTextPair> Layout::layout(const std::vector<Token>& tokens,
                                             sf::Font& font, int window_width) {
  std::vector<PositionTextPair> display_list;

  const float line_space = font.getLineSpacing(16) * 1.25f;
  const float space_width = font.getGlyph(' ', 16, false).advance;

  for (const auto& token : tokens) {
  }

  return display_list;
}

void Layout::token(const auto& token) {
  if (std::holds_alternative<Text>(token)) {
    const auto& text = std::get<Text>(token);
    std::istringstream stream(text.text);
    std::string word;

    while (stream >> word) {
      sf::String sf_word = sf::String::fromUtf8(word.begin(), word.end());
      sf::Text wrd(sf_word, font, 16);

      sf::Uint32 sfml_style = sf::Text::Regular;

      if (m_weight == "bold") {
        sfml_style |= sf::Text::Bold;
      }
      if (m_style == "italic") {
        sfml_style |= sf::Text::Italic;
      }
      wrd.setStyle(sfml_style);
      const auto word_width = wrd.getLocalBounds().width;

      LayoutElement element{.type = LayoutElementType::TEXT, .value = sf_word};

      if (!sf_word.isEmpty() && common::isEmoji(sf_word[0])) {
        element.type = LayoutElementType::EMOJI;
      }
      if (m_cursor_x + word_width > window_width - HSTEP) {
        m_cursor_y += line_space;
        m_cursor_x = HSTEP;
      }

      display_list.emplace_back(m_cursor_x, m_cursor_y, element, wrd);
      m_cursor_x += word_width + space_width;
    }
  } else {
    const auto& tag = std::get<Tag>(token).tag;
    if (tag == "i") {
      m_style = "italic";
    } else if (tag == "/i") {
      m_style = "roman";
    } else if (tag == "b") {
      m_weight = "bold";
    } else if (tag == "/b") {
      m_weight = "normal";
    }
  }
}
}  // namespace layout
