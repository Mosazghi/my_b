#include "utils.h"
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

}  // namespace utils
