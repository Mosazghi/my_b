#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <vector>
namespace utils {
std::vector<std::string> split_string(const std::string& s, char delim);

void ltrim(std::string& s);

// Trim from end (in place)
void rtrim(std::string& s);

// Trim from both ends (in place)
void trim(std::string& s);

std::optional<std::string> ungzip(const std::string& compressed);

}  // namespace utils
