#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <vector>
namespace utils {
/**
 * @brief Split a string by a delimiter
 * @param s String to split
 * @param delim Delimiter character
 * @return std::vector<std::string> Vector of splitted strings
 */
std::vector<std::string> split_string(const std::string& s, char delim);

/**
 * @brief Trim whitespace from both ends of the string
 * @param s String to trim (in place)
 */
void trim(std::string& s);

/**
 * @brief Decompress a gzip compressed string
 * @param compressed
 * @return std::optional<std::string> Decompressed string if successful,
 * std::nullopt otherwise
 */
std::optional<std::string> ungzip(std::string_view compressed);

}  // namespace utils
