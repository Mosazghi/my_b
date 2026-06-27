#include "layout.hpp"
#include <cmath>
#include <format>
#include <iostream>
#include <sstream>
#include "common.hpp"

namespace layout {

static void process_word(LayoutContext& ctx, const std::string& word);
static void process_tag(LayoutContext& ctx, const std::string& tag);
static void process_token(LayoutContext& ctx, const Token& token);
static void flush_line(LayoutContext& ctx);

static void process_token(LayoutContext& ctx, const Token& token) {
  if (std::holds_alternative<Text>(token)) {
    const auto& text = std::get<Text>(token);
    std::istringstream stream(text.text);
    std::string word;

    while (stream >> word) {
        process_word(ctx, word);
    }
  } else {
    const auto& tag = std::get<Tag>(token).tag;
    process_tag(ctx, tag);
  }
}

static void process_word(LayoutContext& ctx, const std::string& word) {
    const float space_width = ctx.font.getGlyph(' ', ctx.size, false).advance;

    sf::String sf_word = sf::String::fromUtf8(word.begin(), word.end());
    sf::Text font(sf_word, ctx.font, ctx.size);

    sf::Uint32 sfml_style = sf::Text::Regular;

    if (ctx.weight == "bold") {
      sfml_style |= sf::Text::Bold;
    }
    if (ctx.style == "italic") {
      sfml_style |= sf::Text::Italic;
    }
    font.setStyle(sfml_style);

    const auto word_width = font.getLocalBounds().width;

    LayoutElement element{.type = LayoutElementType::TEXT, .value = sf_word};

    if (!sf_word.isEmpty() && common::isEmoji(sf_word[0])) {
      element.type = LayoutElementType::EMOJI;
    }
    if (ctx.cursor_x + word_width >= ctx.window_width - HSTEP) {
        flush_line(ctx);
    }

    ctx.line.emplace_back(ctx.cursor_x, element, font);
    ctx.cursor_x += word_width + space_width;
}

static void process_tag(LayoutContext& ctx, const std::string& tag) {
    if (tag == "i") {
      ctx.style = "italic";
    } else if (tag == "/i") {
      ctx.style = "roman";
    } else if (tag == "b") {
      ctx.weight = "bold";
    } else if (tag == "/b") {
      ctx.weight = "normal";
    } else if (tag == "br") {
      flush_line(ctx);
    } else if (tag == "/p") {
      flush_line(ctx);
      ctx.cursor_y += VSTEP;
    } else if (tag == "small") {
        ctx.size -= 2;
    } else if (tag == "/small") {
        ctx.size += 2;
    } else if (tag == "big") {
        ctx.size += 4;
    } else if (tag == "/big") {
        ctx.size -= 4;
    }
}

static void flush_line(LayoutContext& ctx) {
    if (ctx.line.empty()) {
       return;
    }

    std::vector<std::tuple<float, float>> metrics(ctx.line.size());

    for (size_t i = 0; i < ctx.line.size(); ++i) {
        // sfml v2 "way" of propely getting necessary metrics
        const sf::Text&   text = std::get<2>(ctx.line[i]);
        const sf::String& word = std::get<1>(ctx.line[i]).value;
        const unsigned int size = text.getCharacterSize();
        const bool         bold = (text.getStyle() & sf::Text::Bold) != 0;

        float ascent = 0.f;
        float descent = 0.f;
        for (std::size_t k = 0; k < word.getSize(); ++k) {
            const sf::Glyph& g = ctx.font.getGlyph(word[k], size, bold);
            ascent  = std::max(ascent,  -g.bounds.top);
            descent = std::max(descent,  g.bounds.top + g.bounds.height);
        }
        metrics[i] = std::make_tuple(ascent, descent);
    }

    auto max_ascent = -std::numeric_limits<float>::infinity();
    for (const auto& [ascent, descent] : metrics) {
        max_ascent = std::max(max_ascent, ascent);
    }


    const float baseline = ctx.cursor_y + 1.25f * max_ascent;

    size_t i = 0;
    for (const auto& [x, element, font] : ctx.line) {
        const auto y = baseline - std::get<0>(metrics[i]);
        ctx.display_list.emplace_back(x, y, element, font);
        i++;
    }


    auto max_descent = -std::numeric_limits<float>::infinity();
    for (const auto& [ascent, descent] : metrics) {
        max_descent = std::max(max_descent, descent);
    }
    ctx.cursor_y = baseline + 1.25f * max_descent;

    ctx.cursor_x = HSTEP;
    ctx.line.clear();
}

std::vector<PositionTextPair> compute(const std::vector<Token>& tokens,
                                      sf::Font& font, int window_width) {
  LayoutContext ctx{};
  ctx.window_width = window_width;
  ctx.font = font;

  for (const auto& token : tokens) {
    process_token(ctx, token);
  }
  flush_line(ctx);
  return ctx.display_list;
}
}  // namespace layout
