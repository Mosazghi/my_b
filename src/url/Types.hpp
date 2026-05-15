#pragma once
#include <cstdint>
namespace url {

enum class Scheme : std::uint8_t {
  UNKNOWN,  //< Unsupported.
  HTTP,
  HTTPS,
  FILE,
  DATA,
  VIEW_SOURCE
};
}
