#include "layout.hpp"
#include <sstream>
#include "common.hpp"

namespace layout {

static void process_token(LayoutContext& ctx, const Token& token,
                          sf::Font& font, int window_width) {
  const float line_space = font.getLineSpacing(16) * 1.25f;
  const float space_width = font.getGlyph(' ', 16, false).advance;
  if (std::holds_alternative<Text>(token)) {
    const auto& text = std::get<Text>(token);
    std::istringstream stream(text.text);
    std::string word;

    while (stream >> word) {
      sf::String sf_word = sf::String::fromUtf8(word.begin(), word.end());
      sf::Text wrd(sf_word, font, 16);

      sf::Uint32 sfml_style = sf::Text::Regular;

      if (ctx.weight == "bold") {
        sfml_style |= sf::Text::Bold;
      }
      if (ctx.style == "italic") {
        sfml_style |= sf::Text::Italic;
      }
      wrd.setStyle(sfml_style);
      const auto word_width = wrd.getLocalBounds().width;

      LayoutElement element{.type = LayoutElementType::TEXT, .value = sf_word};

      if (!sf_word.isEmpty() && common::isEmoji(sf_word[0])) {
        element.type = LayoutElementType::EMOJI;
      }
      if (ctx.cursor_x + word_width > window_width - HSTEP) {
        ctx.cursor_y += line_space;
        ctx.cursor_x = HSTEP;
      }

      ctx.display_list.emplace_back(ctx.cursor_x, ctx.cursor_y, element, wrd);
      ctx.cursor_x += word_width + space_width;
    }
  } else {
    const auto& tag = std::get<Tag>(token).tag;
    if (tag == "i") {
      ctx.style = "italic";
    } else if (tag == "/i") {
      ctx.style = "roman";
    } else if (tag == "b") {
      ctx.weight = "bold";
    } else if (tag == "/b") {
      ctx.weight = "normal";
    }
  }
}

std::vector<PositionTextPair> compute(const std::vector<Token>& tokens,
                                      sf::Font& font, int window_width) {
  LayoutContext ctx{};

  for (const auto& token : tokens) {
    process_token(ctx, token, font, window_width);
  }

  return ctx.display_list;
}
}  // namespace layout
