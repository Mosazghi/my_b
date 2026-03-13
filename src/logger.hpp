#pragma once
#include <chrono>
#include <format>
#include <iostream>
#include <mutex>
#include <source_location>
#include <string_view>

enum class LogLevel { DBG, INFO, WARN, ERROR };

class Logger {
 public:
  /**
   * @brief Get the singleton instance of the Logger
   */
  static Logger& getInstance() {
    static Logger instance;
    return instance;
  }

  struct LogFormat {
    std::string_view fmt;
    std::source_location loc;

    template <typename T>
    consteval LogFormat(
        const T& s, std::source_location l = std::source_location::current())
        : fmt(s), loc(l) {}
  };

  template <typename... Args>
  void err(LogFormat format, Args&&... args) {
    log(LogLevel::ERROR, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void warn(LogFormat format, Args&&... args) {
    log(LogLevel::WARN, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void inf(LogFormat format, Args&&... args) {
    log(LogLevel::INFO, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void dbg(LogFormat format, Args&&... args) {
#if defined(DEBUG) || defined(_DEBUG)
    log(LogLevel::DBG, format, std::forward<Args>(args)...);
#endif
  }

 private:
  // Enforce singleton pattern
  Logger() = default;
  ~Logger() = default;
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(Logger&&) = delete;

  template <typename... Args>
  void log(LogLevel lvl, const LogFormat& format, Args&&... args) {
    const auto time =
        std::chrono::current_zone()->to_local(std::chrono::system_clock::now());

    const std::string user_msg =
        std::vformat(format.fmt, std::make_format_args(args...));

    const std::string_view short_file_name =
        get_short_file_name(format.loc.file_name());

    const std::string final_output =
        std::format("{:%Y-%m-%d %H:%M:%S} {}{:5}{} {}:{}: {}\n", time,
                    lvl_to_color_code(lvl), lvl_to_string(lvl), "\033[0m",
                    short_file_name, format.loc.line(), user_msg);

    {
      std::lock_guard lock(m_mutex);
      std::cerr << final_output;
    }
  }

  static constexpr std::string_view get_short_file_name(
      const std::string_view& full_file_path) {
    size_t sep_pos = full_file_path.rfind("/");

    if (sep_pos != std::string::npos) {
      return full_file_path.substr(sep_pos + 1);
    } else {
      return full_file_path;
    }
  }

  static constexpr std::string_view lvl_to_color_code(LogLevel lvl) {
    switch (lvl) {
      case LogLevel::DBG:
        return "\033[36m";
      case LogLevel::INFO:
        return "\033[32m";
      case LogLevel::WARN:
        return "\033[33m";
      case LogLevel::ERROR:
        return "\033[31m";
      default:
        return "\033[32m";
    }
  }

  static constexpr std::string_view lvl_to_string(LogLevel lvl) {
    switch (lvl) {
      case LogLevel::DBG:
        return "DEBUG";
      case LogLevel::INFO:
        return "INFO ";
      case LogLevel::WARN:
        return "WARN ";
      case LogLevel::ERROR:
        return "ERROR";
      default:
        return "UNKNOWN";
    }
  }

  std::mutex m_mutex;
};
