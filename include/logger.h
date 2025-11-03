#pragma once
#include <chrono>
#include <cstdarg>
#include <format>
#include <iostream>
#define BASE_LOG(fmt, info, ...) printf(info fmt + '\n', ##__VA_ARGS__);
#if DEBUG
#define LOG_DBG(fmt, ...) BASE_LOG(fmt, "[Debug]: ", ##__VA_ARGS__);
#else
#define LOG_DBG(fmt, ...) \
  do {                    \
  } while (0);
#endif
#define LOG_INF(fmt, ...) BASE_LOG(fmt, "[Info]: ", ##__VA_ARGS__);
#define LOG_ERR(fmt, ...) BASE_LOG(fmt, "[Error]: ", ##__VA_ARGS__);

enum LogLevel { INFO, DEBUG, ERROR };
class Logger {
 public:
  Logger(std::string_view name) : m_name(name) {}
  ~Logger() {}
  template <typename... Args>
  void err(std::string_view fmt, Args&&... args) {
    base(fmt, LogLevel::ERROR, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void dbg(std::string_view fmt, Args&&... args) {
    base(fmt, LogLevel::DEBUG, std::forward<Args>(args)...);
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
    std::string const formatted =
        std::format("{} - {}:{} - {} - {}\n", buffer, m_name, __LINE__,
                    lvl_to_string(lvl), msg);

    switch (lvl) {
      case DEBUG:
      case ERROR:
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
      case DEBUG:
        return "INFO";
        break;
      case ERROR:
        return "ERROR";
        break;
      case INFO:
        return "INFO";
        break;
      default:
        return "UNKNOWN";
        break;
    }
  }
  std::string m_name;
};
