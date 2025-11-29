#pragma once
#include <optional>
#include <string>

namespace file {
std::optional<std::string> read(const std::string& path);
}  // namespace file
