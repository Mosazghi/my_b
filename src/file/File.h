#pragma once
#include <optional>
#include <string>

namespace file {
/**
 * @brief Reads the content of a file at the given path
 * @param path Path to the file
 * @return std::optional<std::string> Content of the file if successful, empty
 * optional otherwise
 */
std::optional<std::string> read(const std::string& path);
}  // namespace file
