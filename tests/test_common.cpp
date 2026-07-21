#include <gtest/gtest.h>
#include <string>
#include "common/common.hpp"

using namespace my_b;
using namespace my_b::layout;

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

TEST(CommonLex, TrimsSurroundingWhitespaceFromText) {
  std::string body = "<p>  padded text  </p>";
  auto tokens = common::lex(body);

  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(std::get<Text>(tokens[1]).text, "padded text");
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
