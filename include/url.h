#pragma once

#include <string>

class URL {
 public:
  URL(std::string const& url);
  void request();

 private:
  std::string m_scheme{};
  std::string m_hostname{};
  std::string m_path{};
  int m_port;
};
