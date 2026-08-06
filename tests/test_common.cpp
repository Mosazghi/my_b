#include <gtest/gtest.h>
#include <string>
#include <variant>
#include <vector>
#include "common/common.hpp"

using namespace my_b;
using namespace my_b::layout;

namespace {

// Renders a token stream as "tag:<name>@<parent>" / "text:<content>" so a whole
// document can be asserted in one comparison with a readable diff on failure
std::vector<std::string> summarize(const std::vector<Token>& tokens) {
  std::vector<std::string> out;
  out.reserve(tokens.size());
  for (const auto& token : tokens) {
    std::visit(
        [&](const auto& t) {
          using T = std::decay_t<decltype(t)>;
          if constexpr (std::is_same_v<T, Text>) {
            out.push_back("text:" + t.text);
          } else {
            out.push_back("tag:" + t.tag + "@" + t.parent_tag);
          }
        },
        token);
  }
  return out;
}

std::vector<std::string> summarize_tags(const std::vector<Token>& tokens) {
  std::vector<std::string> out;
  for (const auto& token : tokens) {
    if (const auto* tag = std::get_if<Tag>(&token)) {
      out.push_back("tag:" + tag->tag + "@" + tag->parent_tag);
    }
  }
  return out;
}

}  // namespace

TEST(CommonLex, PlainText) {
  std::string body = "Hello world";
  auto tokens = common::lex(body);

  ASSERT_EQ(tokens.size(), 1);
  ASSERT_TRUE(std::holds_alternative<Text>(tokens[0]));
  EXPECT_EQ(std::get<Text>(tokens[0]).text, "Hello world");
}

TEST(CommonLex, EmptyInput) {
  std::string body = "";
  auto tokens = common::lex(body);
  EXPECT_TRUE(tokens.empty());
}

TEST(CommonLex, SimpleTagPair) {
  std::string body = "<b>bold</b>";
  auto tokens = common::lex(body);

  ASSERT_EQ(tokens.size(), 3);
  ASSERT_TRUE(std::holds_alternative<Tag>(tokens[0]));
  EXPECT_EQ(std::get<Tag>(tokens[0]).tag, "b");

  ASSERT_TRUE(std::holds_alternative<Text>(tokens[1]));
  EXPECT_EQ(std::get<Text>(tokens[1]).text, "bold");

  ASSERT_TRUE(std::holds_alternative<Tag>(tokens[2]));
  EXPECT_EQ(std::get<Tag>(tokens[2]).tag, "/b");
}

TEST(CommonLex, TagWithAttributesSplitsNameFromRest) {
  std::string body = "<div class=\"text-center\">Hi</div>";
  auto tokens = common::lex(body);

  ASSERT_EQ(tokens.size(), 3);
  const auto& open_tag = std::get<Tag>(tokens[0]);
  EXPECT_EQ(open_tag.tag, "div");
  EXPECT_NE(open_tag.rest.find("text-center"), std::string::npos);
}

TEST(CommonLex, DecodesHtmlEntitiesBeforeTokenizing) {
  std::string body = "&lt;b&gt;bold&lt;/b&gt;";
  auto tokens = common::lex(body);

  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(std::get<Tag>(tokens[0]).tag, "b");
  EXPECT_EQ(std::get<Text>(tokens[1]).text, "bold");
  EXPECT_EQ(std::get<Tag>(tokens[2]).tag, "/b");
}

TEST(CommonLex, NestedInlineMarkupTracksImmediateParent) {
  std::string body =
      "<html><body><h1>Title</h1><p>Some <b>bold</b> text</p></body></html>";

  EXPECT_EQ(summarize(common::lex(body)),
            (std::vector<std::string>{
                "tag:html@", "tag:body@html", "tag:h1@body", "text:Title",
                "tag:/h1@body", "tag:p@body", "text:Some", "tag:b@p",
                "text:bold", "tag:/b@p", "text:text", "tag:/p@body",
                "tag:/body@html", "tag:/html@"}));
}

TEST(CommonLex, SiblingSubtreesUnwindTheTagStack) {
  std::string body =
      "<ul><li><a href=\"#one\">one</a></li><li><a href=\"#two\">two</a></li>"
      "</ul>";

  EXPECT_EQ(
      summarize_tags(common::lex(body)),
      (std::vector<std::string>{"tag:ul@", "tag:li@ul", "tag:a@li", "tag:/a@a",
                                "tag:/li@li", "tag:li@ul", "tag:a@li",
                                "tag:/a@a", "tag:/li@li", "tag:/ul@ul"}));
}

TEST(CommonLex, VoidElementsNeverBecomeParents) {
  std::string body = "<body><br><img src=\"a.png\"><p>after</p></body>";

  EXPECT_EQ(
      summarize_tags(common::lex(body)),
      (std::vector<std::string>{"tag:body@", "tag:br@body", "tag:img@body",
                                "tag:p@body", "tag:/p@p", "tag:/body@body"}));
}

TEST(CommonLex, SelfClosingTagOutsideVoidListIsNotPushed) {
  std::string body = "<section><widget id=\"1\" /><span>x</span></section>";
  auto tokens = common::lex(body);

  EXPECT_EQ(summarize_tags(tokens),
            (std::vector<std::string>{"tag:section@", "tag:widget@section",
                                      "tag:span@section", "tag:/span@span",
                                      "tag:/section@section"}));
}

TEST(CommonLex, DoctypeIsVoidAndKeepsItsRest) {
  std::string body = "<!DOCTYPE html><html><head></head></html>";
  auto tokens = common::lex(body);

  const auto& doctype = std::get<Tag>(tokens[0]);
  EXPECT_EQ(doctype.tag, "!DOCTYPE");
  EXPECT_EQ(doctype.rest, " html");
  // <html> would report the doctype as its parent if it had been pushed.
  EXPECT_EQ(std::get<Tag>(tokens[1]).parent_tag, "");
}

TEST(CommonLex, AttributeRestIsPreservedVerbatimWithLeadingSpace) {
  std::string body =
      R"(<a href="https://example.com" target="_blank">link</a>)";
  auto tokens = common::lex(body);

  const auto& anchor = std::get<Tag>(tokens[0]);
  EXPECT_EQ(anchor.tag, "a");
  EXPECT_EQ(anchor.rest, R"( href="https://example.com" target="_blank")");
  EXPECT_EQ(std::get<Tag>(tokens[2]).rest, "");
}

TEST(CommonLex, IndentedMarkupEmitsEmptyTextForInterTagWhitespace) {
  std::string body =
      "<div>\n"
      "  <p>Hello</p>\n"
      "</div>";

  EXPECT_EQ(
      summarize(common::lex(body)),
      (std::vector<std::string>{"tag:div@", "text:", "tag:p@div", "text:Hello",
                                "tag:/p@p", "text:", "tag:/div@div"}));
}

TEST(CommonLex, UnclosedTagsKeepNesting) {
  std::string body = "<p>one<p>two";

  EXPECT_EQ(
      summarize(common::lex(body)),
      (std::vector<std::string>{"tag:p@", "text:one", "tag:p@p", "text:two"}));
}

TEST(CommonLex, StrayClosingTagsDoNotUnderflowTagStack) {
  std::string body = "</p></div><b>x</b>";

  EXPECT_EQ(summarize(common::lex(body)),
            (std::vector<std::string>{"tag:/p@", "tag:/div@", "tag:b@",
                                      "text:x", "tag:/b@b"}));
}

TEST(CommonLex, RealisticDocumentStructure) {
  std::string body =
      "<!DOCTYPE html>\n"
      "<html lang=\"en\">\n"
      "  <head>\n"
      "    <meta charset=\"utf-8\">\n"
      "    <title>Example</title>\n"
      "  </head>\n"
      "  <body>\n"
      "    <h1>Heading</h1>\n"
      "    <p>Paragraph with <b>bold</b> and <i>italic</i>.</p>\n"
      "    <hr />\n"
      "  </body>\n"
      "</html>";
  auto tokens = common::lex(body);

  EXPECT_EQ(summarize_tags(tokens),
            (std::vector<std::string>{
                "tag:!DOCTYPE@", "tag:html@", "tag:head@html", "tag:meta@head",
                "tag:title@head", "tag:/title@title", "tag:/head@head",
                "tag:body@html", "tag:h1@body", "tag:/h1@h1", "tag:p@body",
                "tag:b@p", "tag:/b@b", "tag:i@p", "tag:/i@i", "tag:/p@p",
                "tag:hr@body", "tag:/body@body", "tag:/html@html"}));

  std::vector<std::string> text;
  for (const auto& token : tokens) {
    if (const auto* t = std::get_if<Text>(&token); t && !t->text.empty()) {
      text.push_back(t->text);
    }
  }
  EXPECT_EQ(text,
            (std::vector<std::string>{"Example", "Heading", "Paragraph with",
                                      "bold", "and", "italic", "."}));
}

TEST(CommonEmoji, DetectsEmojiCodepoint) {
  EXPECT_TRUE(common::isEmoji(0x1F600));  // grinning face
}

TEST(CommonEmoji, RegularLetterIsNotEmoji) {
  EXPECT_FALSE(common::isEmoji('A'));
}

TEST(CommonEmoji, GetEmojiIdFormatsAsPaddedUppercaseHex) {
  EXPECT_EQ(common::get_emoji_id(0x1F600), "1F600");
  EXPECT_EQ(common::get_emoji_id('A'), "00041");
}
