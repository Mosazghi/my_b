#include "File.h"
#include <fstream>
#include <optional>
#include <string>
#include "logger.h"

namespace file {

static Logger& logger = Logger::getInstance();

std::optional<std::string> read(const std::string& path) {
  std::ifstream file(path);

  if (!file.is_open()) {
    logger.warn("Could not open the file '{}'", path);
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
