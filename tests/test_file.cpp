#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "file/File.h"
#include "test_helpers.h"

TEST(FileTest, ReadValidFiles) {
  std::string file_path = get_mock_data_file_path("/mock_data/short-file.txt");

  auto content = file::read(file_path);
  EXPECT_TRUE(content.has_value());
  auto first_line =
      content.value().substr(0, content.value().find_first_of("\n"));
  EXPECT_EQ(first_line, "This is a short file.");

  file_path = get_mock_data_file_path("/mock_data/simple-html.html");
  content = file::read(file_path);

  first_line = content.value().substr(0, content.value().find_first_of("\n"));
  EXPECT_TRUE(content.has_value());
  EXPECT_EQ(first_line, "<!doctype html>");
}

TEST(FileTest, ReadNonExistentFile) {
  auto content = file::read("/nonexistent/path/file.txt");
  EXPECT_FALSE(content.has_value());
}
