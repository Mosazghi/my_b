#include "layout.hpp"
#include "common.hpp"
#include <cmath>
#include <iostream>
#include <sstream>
#include "resource-manager/ResourceManager.h"

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
  const bool has_bold = ctx.weight == "bold";
  const float space_width = ctx.font.getGlyph(' ', ctx.size, has_bold).advance;

  const auto& [text, sf_word] = resource::ResourceManager::get_font(word, ctx);

  const auto word_width = text.getLocalBounds().width;

  LayoutElement element{.type = LayoutElementType::TEXT, .value = sf_word};

  if (!sf_word.isEmpty() && common::isEmoji(sf_word[0])) {
    element.type = LayoutElementType::EMOJI;
  }
  if (ctx.cursor_x + word_width >= ctx.window_width - HSTEP) {
    flush_line(ctx);
  }

  ctx.line.emplace_back(ctx.cursor_x, element, text);
  ctx.cursor_x += word_width + space_width;
}

static void process_tag(LayoutContext& ctx, const std::string& tag) {
  using Action = void (*)(LayoutContext&);

  static const std::unordered_map<std::string, Action> tag_actions = {
      {"i", [](LayoutContext& c) { c.style = "italic"; }},
      {"/i", [](LayoutContext& c) { c.style = "roman"; }},
      {"b", [](LayoutContext& c) { c.weight = "bold"; }},
      {"/b", [](LayoutContext& c) { c.weight = "normal"; }},
      {"br", [](LayoutContext& c) { flush_line(c); }},
      {"/p",
       [](LayoutContext& c) {
         flush_line(c);
         c.cursor_y += VSTEP;
       }},
      {"small", [](LayoutContext& c) { c.size -= 2; }},
      {"/small", [](LayoutContext& c) { c.size += 2; }},
      {"big", [](LayoutContext& c) { c.size += 4; }},
      {"/big", [](LayoutContext& c) { c.size -= 4; }},
  };

  if (const auto it = tag_actions.find(tag); it != tag_actions.end()) {
    it->second(ctx);
  }
}

static void flush_line(LayoutContext& ctx) {
  if (ctx.line.empty()) {
    return;
  }

  std::vector<std::tuple<float, float>> metrics(ctx.line.size());

  for (size_t i = 0; i < ctx.line.size(); ++i) {
    // sfml v2 "way" of propely getting necessary metrics
    const sf::Text& text = std::get<2>(ctx.line[i]);
    const sf::String& word = std::get<1>(ctx.line[i]).value;
    const unsigned int size = text.getCharacterSize();
    const bool bold = (text.getStyle() & sf::Text::Bold) != 0;

    float ascent = 0.f;
    float descent = 0.f;
    for (std::size_t k = 0; k < word.getSize(); ++k) {
      const sf::Glyph& g = ctx.font.getGlyph(word[k], size, bold);
      ascent = std::max(ascent, -g.bounds.top);
      descent = std::max(descent, g.bounds.top + g.bounds.height);
    }
    metrics[i] = std::make_tuple(ascent, descent);
  }

  auto max_ascent = -std::numeric_limits<float>::infinity();
  for (const auto& [ascent, descent] : metrics) {
    max_ascent = std::max(max_ascent, ascent);
  }

  const float baseline = ctx.cursor_y + 1.25f * max_ascent;

  size_t i = 0;
  for (const auto& [x, element, text] : ctx.line) {
    const auto y = baseline - std::get<0>(metrics[i]);
    ctx.display_list.emplace_back(x, y, element, text);
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
  LayoutContext ctx{
      .window_width = window_width,
      .font = font,
  };

  for (const auto& token : tokens) {
    process_token(ctx, token);
  }
  flush_line(ctx);
  return ctx.display_list;
}
}  // namespace layout
