#include "layout.hpp"
#include <fmt/base.h>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Glyph.hpp>
#include <SFML/Graphics/Text.hpp>
#include <algorithm>
#include <cstdint>
#include <optional>
#include <sstream>
#include "common/common.hpp"
#include "logger.hpp"
#include "resource-manager/ResourceManager.h"
using namespace my_b;

namespace my_b::layout {

static void process_word(LayoutContext& ctx, const std::string& word);
static void process_tag(LayoutContext& ctx, const std::string& tag);
static void process_token(LayoutContext& ctx, const Token& token);
static void process_spaces(LayoutContext& ctx, const int num_spaces);

static void flush_line(LayoutContext& ctx);
static auto& logger = Logger::getInstance();

static void process_abbr(LayoutContext& ctx, sf::String text,
                         LayoutElement& element, const float space_width) {
  for (auto& c : text) {
    std::uint32_t style{};
    if (c < 128 && std::islower(static_cast<unsigned char>(c))) {
      c = std::toupper(static_cast<unsigned char>(c));
      style |= sf::Text::Bold;
    }
    const bool is_bold = (style & sf::Text::Bold) != 0;

    sf::Text abbr_text(*ctx.font, c, ctx.size);
    abbr_text.setFillColor(sf::Color::Black);
    abbr_text.setStyle(style);
    const auto bounds = abbr_text.getLocalBounds();
    abbr_text.setOrigin(bounds.position);

    element.value = abbr_text.getString();
    ctx.line.emplace_back(ctx.cursor_x, element, abbr_text);
    ctx.cursor_x += ctx.font->getGlyph(c, ctx.size, is_bold).advance;
  }
  ctx.cursor_x += space_width;
}

static void process_token(LayoutContext& ctx, const Token& token) {
  if (std::holds_alternative<Text>(token)) {
    const auto& text = std::get<Text>(token);
    std::istringstream stream(text.text);
    char ch{};
    std::string word{};

    const auto prepare_word = [&]() -> void {
      process_word(ctx, word);
      word.clear();
    };

    while (stream.get(ch)) {
      if (ch == ' ') {
        prepare_word();
        std::string spaces;

        while (stream.peek() != EOF && std::isspace(stream.peek())) {
          spaces += stream.get();
        }
        if (!spaces.empty() && ctx.has_tag("pre")) {
          process_spaces(ctx, spaces.length());
        }
      } else if (ch == '\n') {
        bool is_pre_tag = ctx.has_tag("pre");
        if (is_pre_tag) {
          prepare_word();
          flush_line(ctx);
        }
      } else {
        word += ch;
      }
    }
    prepare_word();
  } else {
    static std::vector<Tag> tags{};
    Tag tag_token = std::get<Tag>(token);
    if (tag_token.is_closing()) {
      tags.pop_back();
    } else {
      tags.push_back(tag_token);
    }
    ctx.current_tag = tags.back();
    process_tag(ctx, tag_token.tag);
  }
}

static void process_spaces(LayoutContext& ctx, const int num_spaces) {
  const auto space_char{" "};
  auto [text, sf_word] = resource::ResourceManager::get_font(space_char, ctx);
  for (auto i{0}; i < num_spaces; ++i) {
    const auto text_width{text.getLocalBounds().size.x};
    ctx.cursor_x += text_width;

    LayoutElement elem{
        .type = LayoutElementType::Text,
        .value = space_char,
        .tag = *ctx.current_tag,
    };

    ctx.display_content.emplace_back(ctx.cursor_x, ctx.cursor_y, elem, text);
  }
}

static void process_word(LayoutContext& ctx, const std::string& word) {
  if (word.empty()) {
    return;
  }
  if (ctx.current_tag && ctx.current_tag->parent_tag == "head") {
    return;
  }

  const bool has_bold = ctx.weight == "bold";
  const float space_width = ctx.font->getGlyph(' ', ctx.size, has_bold).advance;

  auto [text, sf_word] = resource::ResourceManager::get_font(word, ctx);

  auto word_width = text.getLocalBounds().size.x;

  LayoutElement element{.type = LayoutElementType::Text,
                        .vertical_align = ctx.vertical_align,
                        .value = sf_word,
                        .tag = ctx.current_tag
                                   ? std::make_optional(*ctx.current_tag)
                                   : std::nullopt};

  if (!sf_word.isEmpty() && common::isEmoji(sf_word[0])) {
    element.type = LayoutElementType::Emoji;
  }

  if (ctx.cursor_x + word_width >= ctx.window_width - HSTEP) {
    flush_line(ctx);
  }

  if (ctx.line.size() > 0 &&
      std::get<1>(ctx.line.back()).type == LayoutElementType::Emoji) {
    ctx.cursor_x += 8;
  }

  bool is_abbr = ctx.has_tag("abbr");
  if (is_abbr) {
    process_abbr(ctx, text.getString(), element, space_width);
    return;
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
      {"pre",
       [](LayoutContext& c) {
         auto* courier_font = new sf::Font;
         if (!courier_font->openFromFile("assets/Courier-New-Regular.ttf")) {
           logger.err("Error loading font\n");
         }
         c.font = courier_font;
       }},
      {"/pre",
       [](LayoutContext& c) {
         c.font = c.default_font;
         if (c.line.empty()) {
           c.cursor_x = HSTEP;
         }
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
      {"sup",
       [](LayoutContext& c) {
         c.size -= 3;
         c.vertical_align = VerticalAlign::Super;
       }},
      {"/sup",
       [](LayoutContext& c) {
         c.size += 3;
         c.vertical_align = VerticalAlign::Baseline;
       }},

      {"sub",
       [](LayoutContext& c) {
         c.size -= 3;
         c.vertical_align = VerticalAlign::Sub;
       }},
      {"/sub",
       [](LayoutContext& c) {
         c.size += 3;
         c.vertical_align = VerticalAlign::Baseline;
       }},

      {"abbr", [](LayoutContext& c) { c.size -= 5; }},
      {"/abbr", [](LayoutContext& c) { c.size += 5; }},
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
    const sf::Text& text = std::get<2>(ctx.line[i]);
    const sf::String& word = std::get<1>(ctx.line[i]).value;
    const unsigned int size = text.getCharacterSize();
    const bool bold = (text.getStyle() & sf::Text::Bold) != 0;

    float ascent = 0.f;
    float descent = 0.f;
    for (std::size_t k = 0; k < word.getSize(); ++k) {
      const sf::Glyph& g = ctx.font->getGlyph(word[k], size, bold);
      ascent = std::max(ascent, -g.bounds.position.y);
      descent = std::max(descent, g.bounds.position.y + g.bounds.size.y);
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
        return true;
      }
    }
    return false;
  }();

  const float first_x = std::get<0>(ctx.line.front());
  const float last_x = std::get<0>(ctx.line.back());
  const float last_word_w =
      std::get<2>(ctx.line.back()).getLocalBounds().size.x;
  const float line_width = (last_x + last_word_w) - first_x;

  const float shift = (ctx.window_width - line_width) / 2.0f - first_x;

  size_t j{0};
  for (auto& [x, element, text] : ctx.line) {
    const auto own_ascent = std::get<0>(metrics[j]);
    auto y = baseline - own_ascent;
    if (line_should_be_centered) {
      x += shift;
    }

    if (element.vertical_align == VerticalAlign::Super) {
      y -= (max_ascent - own_ascent) * 1.0f;
      x -= 2.5f;
    } else if (element.vertical_align == VerticalAlign::Sub) {
      y += (max_ascent - own_ascent) * 0.3f;
      x -= 2.5f;
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
                                      sf::Font* font, int window_width) {
  LayoutContext ctx{
      .window_width = window_width,
      .font = font,
      .default_font = font,
  };

  for (const auto& token : tokens) {
    process_token(ctx, token);
  }
  flush_line(ctx);
  return ctx.display_content;
}
}  // namespace my_b::layout
