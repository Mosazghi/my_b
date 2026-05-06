#pragma once

#include <format>
#include <string>

inline std::string get_mock_data_file_path(std::string_view path) {
  return std::format("{}/mock_data/{}", TEST_DATA_DIR, path);
}
