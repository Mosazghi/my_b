#include "utils.h"
#include <zlib.h>
#include <algorithm>
#include <cctype>
#include <optional>
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

std::optional<std::string> ungzip(const std::string& compressed) {
  if (compressed.empty()) return {};

  z_stream strm{};
  strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressed.data()));
  strm.avail_in = static_cast<uInt>(compressed.size());

  if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK) {
    return {};
  }

  std::string out;
  const size_t chunkSize = 16 * 1024;
  int ret;

  do {
    out.resize(out.size() + chunkSize);
    strm.next_out = reinterpret_cast<Bytef*>(&out[out.size() - chunkSize]);
    strm.avail_out = chunkSize;

    ret = inflate(&strm, Z_NO_FLUSH);
    if (ret != Z_OK && ret != Z_STREAM_END) {
      inflateEnd(&strm);
      return {};
    }
  } while (ret != Z_STREAM_END);

  inflateEnd(&strm);
  out.resize(strm.total_out);
  return out;
}

}  // namespace utils
