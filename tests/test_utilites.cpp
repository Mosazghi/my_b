#include <gtest/gtest.h>
#include <zlib.h>
#include <string>
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
