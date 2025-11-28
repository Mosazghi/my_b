#pragma once
namespace url {

enum class Scheme {
  UNKNOWN,  //< Unsupported.
  HTTP,
  HTTPS,
  FILE,
  DATA,
};
}
