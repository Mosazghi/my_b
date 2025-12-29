#include <gtest/gtest.h>
#include <zlib.h>
#include <string>
#include "file/File.h"
#include "utils.h"

TEST(Utils, SplitString) {
  auto str1 = "hello,world";
  auto splitted = utils::split_string(str1, ',');
  EXPECT_EQ(splitted.size(), 2);
  EXPECT_EQ(splitted[0], "hello");
  EXPECT_EQ(splitted[1], "world");
}

TEST(Utils, SplitStringEmptyString) {
  auto str = "";
  auto splitted = utils::split_string(str, ',');
  EXPECT_EQ(splitted.size(), 1);
  EXPECT_EQ(splitted[0], "");
}

TEST(Utils, SplitStringNoDelimiter) {
  auto str = "helloworld";
  auto splitted = utils::split_string(str, ',');
  EXPECT_EQ(splitted.size(), 1);
  EXPECT_EQ(splitted[0], "helloworld");
}

TEST(Utils, SplitStringDelimiterAtStart) {
  auto str = ",hello,world";
  auto splitted = utils::split_string(str, ',');
  EXPECT_EQ(splitted.size(), 3);
  EXPECT_EQ(splitted[0], "");
  EXPECT_EQ(splitted[1], "hello");
  EXPECT_EQ(splitted[2], "world");
}

TEST(Utils, SplitStringDelimiterAtEnd) {
  auto str = "hello,world,";
  auto splitted = utils::split_string(str, ',');
  EXPECT_EQ(splitted.size(), 2);
  EXPECT_EQ(splitted[0], "hello");
  EXPECT_EQ(splitted[1], "world");
}

TEST(Utils, SplitStringConsecutiveDelimiters) {
  auto str = "hello,,world";
  auto splitted = utils::split_string(str, ',');
  EXPECT_EQ(splitted.size(), 3);
  EXPECT_EQ(splitted[0], "hello");
  EXPECT_EQ(splitted[1], "");
  EXPECT_EQ(splitted[2], "world");
}

TEST(Utils, TrimWhitespace) {
  std::string str = "   hello world   ";
  EXPECT_EQ(str, "   hello world   ");
  utils::trim(str);
  EXPECT_EQ(str, "hello world");
}

TEST(Utils, UngzipValidData) {
  const auto file_path = "tests/mock_data/gzipped-data.gz";
  auto gzipped_content_opt = file::read(file_path);

  ASSERT_TRUE(gzipped_content_opt.has_value());
  const auto& gzipped_content = gzipped_content_opt.value();
  auto uncompressed_opt = utils::ungzip(gzipped_content);
  ASSERT_TRUE(uncompressed_opt.has_value());
  const auto& uncompressed = uncompressed_opt.value();
  const std::string expected_content = "Hello World! This is gzipped data\n";
  EXPECT_EQ(uncompressed, expected_content);
}