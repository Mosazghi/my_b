#include "utils.h"
#include <algorithm>  // For std::find_if
#include <cctype>     // For std::isspace
#include <functional>  // For std::not1, std::ptr_fun (C++03, or use lambda in C++11+)
#include <sstream>
#include <string>
#include <vector>

namespace utils {
std::vector<std::string> split_string(const std::string& s, char delim) {
  if (s.empty()) return {""};
  std::vector<std::string> strings;
  std::string temp;
  std::istringstream ss(s);

  while (std::getline(ss, temp, delim)) {
    strings.push_back(temp);
  }

  return strings;
}

// Trim from start (in place)
void ltrim(std::string& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
}

// Trim from end (in place)
void rtrim(std::string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

// Trim from both ends (in place)
void trim(std::string& s) {
  ltrim(s);
  rtrim(s);
}

// Trim from start (copying)
std::string ltrim_copy(std::string s) {
  ltrim(s);
  return s;
}

// Trim from end (copying)
std::string rtrim_copy(std::string s) {
  rtrim(s);
  return s;
}

// Trim from both ends (copying)
std::string trim_copy(std::string s) {
  trim(s);
  return s;
}

}  // namespace utils
