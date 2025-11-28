#pragma once
#include <optional>
#include <string>
#include "logger.h"
namespace file {

class File {
 public:
  File();
  ~File();
  std::optional<std::string> read(const std::string& path);

 private:
  std::string m_path;
  Logger* logger;
};
}  // namespace file
