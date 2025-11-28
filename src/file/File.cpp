#include "File.h"
#include <fstream>
#include <optional>
#include <string>

namespace file {

File::File() { logger = new Logger("FILE"); }
File::~File() {}

std::optional<std::string> File::read(const std::string& path) {
  logger->inf("Reading file {}", path);
  std::ifstream file(path);

  if (!file.is_open()) {
    logger->err("Could not open the file '{}'", path);
    return {};
  }

  std::string line;
  std::string buffer;

  while (std::getline(file, line)) {
    buffer.append(line + "\n");
  }

  file.close();

  return buffer;
}

}  // namespace file
