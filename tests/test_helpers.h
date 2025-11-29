#pragma once

#include <format>
#include <string>

inline std::string get_mock_data_file_path(const std::string& path) {
  return std::format("{}{}", TEST_DATA_DIR, path);
}
