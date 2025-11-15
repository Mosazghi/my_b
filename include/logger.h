#pragma once
#include <chrono>
#include <cstdarg>
#include <format>
#include <iostream>

enum LogLevel { INFO, DBG, WARN, ERROR };
class Logger {
 public:
  Logger(std::string_view name) : m_name(name) {}
  ~Logger() {}

  template <typename... Args>
  void err(std::string_view fmt, Args&&... args) {
    base(fmt, LogLevel::ERROR, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void warn(std::string_view fmt, Args&&... args) {
    base(fmt, LogLevel::WARN, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void dbg(std::string_view fmt, Args&&... args) {
#ifdef DEBUG
    base(fmt, LogLevel::DBG, std::forward<Args>(args)...);
#else
    do {
    } while (0);
#endif
  }
  template <typename... Args>
  void inf(std::string_view fmt, Args&&... args) {
    base(fmt, LogLevel::INFO, std::forward<Args>(args)...);
  }

 private:
  // log_format = "%(asctime)s - %(name)s:%(lineno)d - %(levelname)s -
  // %(message)s" date_format = "%Y-%m-%d %H:%M:%S"
  template <typename... Args>
  inline void base(std::string_view fmt, LogLevel lvl, Args&&... args) {
    auto now = std::chrono::system_clock::now();
    std::time_t current_time = std::chrono::system_clock::to_time_t(now);
  std:
    tm* localtime = std::localtime(&current_time);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime);

    std::string msg = std::vformat(fmt, std::make_format_args(args...));

    // ANSI color codes
    const char* color = "";
    switch (lvl) {
      case WARN:
        color = "\033[35m";
        break;
      case DBG:
        color = "\033[36m";  // Cyan
        break;
      case ERROR:
        color = "\033[31m";  // Red
        break;
      case INFO:
        color = "\033[32m";  // Green
        break;
      default:
        color = "\033[0m";  // Reset
        break;
    }
    const char* reset = "\033[0m";

    std::string const formatted =
        std::format("{} - {}:{} - {} - {}\n", buffer, m_name, __LINE__,
                    color + lvl_to_string(lvl) + reset, msg);

    switch (lvl) {
      case DBG:
      case ERROR:
      case WARN:
        std::cerr << formatted;
        break;
      case INFO:
        std::cout << formatted;
        break;
      default:
        break;
    }
  }
  std::string lvl_to_string(LogLevel lvl) {
    switch (lvl) {
      case DBG:
        return "DEBUG";
      case INFO:
        return "INFO";
      case ERROR:
        return "ERROR";
      case WARN:
        return "WARNING";
      default:
        return "UNKNOWN";
    }
  }
  std::string m_name;
};
