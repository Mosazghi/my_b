#include "layout.hpp"
#include <cmath>
#include <iostream>
#include <sstream>
#include "common.hpp"
#include "imgui.h"
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
    auto& tag_token = std::get<Tag>(token);
    ctx.current_tag = &tag_token;
    process_tag(ctx, tag_token.tag);
  }
}

static void process_word(LayoutContext& ctx, const std::string& word) {
  const bool has_bold = ctx.weight == "bold";
  const float space_width = ctx.font.getGlyph(' ', ctx.size, has_bold).advance;

  auto [text, sf_word] = resource::ResourceManager::get_font(word, ctx);

  auto word_width = text.getLocalBounds().width;

  LayoutElement element{.type = LayoutElementType::TEXT,
                        .value = sf_word,
                        .tag = ctx.current_tag

                                   ? std::make_optional(*ctx.current_tag)

                                   : std::nullopt};

  if (!sf_word.isEmpty() && common::isEmoji(sf_word[0])) {
    element.type = LayoutElementType::EMOJI;
  }
  if (ctx.cursor_x + word_width >= ctx.window_width - HSTEP) {
    flush_line(ctx);
  }
  if (ctx.line.size() > 0 &&
      std::get<1>(ctx.line.back()).type == LayoutElementType::EMOJI) {
    ctx.cursor_x += 8;
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
      {"br/", [](LayoutContext& c) { flush_line(c); }},
      {"/p",
       [](LayoutContext& c) {
         flush_line(c);
         c.cursor_y += VSTEP;
       }},
      {"small", [](LayoutContext& c) { c.size -= 2; }},
      {"/small", [](LayoutContext& c) { c.size += 2; }},
      {"big", [](LayoutContext& c) { c.size += 4; }},
      {"/big", [](LayoutContext& c) { c.size -= 4; }},
      {"h1", [](LayoutContext& c) { c.size += 10; }},
      {"/h1",
       [](LayoutContext& c) {
         c.size -= 10;
         flush_line(c);
       }},
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

  const bool line_should_be_centered = [&]() {
    for (const auto& [x, element, text] : ctx.line) {
      if (element.tag.has_value() &&
          (element.tag->tag == "h1" ||
           element.tag->rest.find("text-center") != std::string::npos)) {
        std::cout << "\tcentered: " << text.getString().toAnsiString()
                  << std::endl;
        return true;
      }
    }
    return false;
  }();
  std::cout << "line_should_be_centered: " << line_should_be_centered
            << std::endl;

  const float first_x = std::get<0>(ctx.line.front());
  const float last_x = std::get<0>(ctx.line.back());
  const float last_word_w = std::get<2>(ctx.line.back()).getLocalBounds().width;
  const float line_width = (last_x + last_word_w) - first_x;

  const float shift = (ctx.window_width - line_width) / 2.0f - first_x;

  size_t j{0};
  for (auto& [x, element, text] : ctx.line) {
    const auto y = baseline - std::get<0>(metrics[j]);
    if (line_should_be_centered) {
      x += shift;
    }
    ctx.display_content.emplace_back(x, y, element, text);
    j++;
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
  return ctx.display_content;
}
}  // namespace layout
