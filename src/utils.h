#pragma once
#include <string>
#include <vector>
namespace utils {
std::vector<std::string> split_string(const std::string& s, char delim);

void ltrim(std::string& s);

// Trim from end (in place)
void rtrim(std::string& s);

// Trim from both ends (in place)
void trim(std::string& s);

}  // namespace utils
