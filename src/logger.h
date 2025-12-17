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
  explicit Logger(std::string_view name) : m_name(name) {}

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
  std::string m_name;
  std::mutex m_mutex;

  template <typename... Args>
  void log(LogLevel lvl, const LogFormat& format, Args&&... args) {
    auto const time =
        std::chrono::current_zone()->to_local(std::chrono::system_clock::now());

    std::string user_msg =
        std::vformat(format.fmt, std::make_format_args(args...));
    std::string_view full_file_path = format.loc.file_name();
    size_t sep_pos = full_file_path.rfind("/");
    std::string_view short_file_name;

    if (sep_pos != std::string::npos) {
      short_file_name = full_file_path.substr(sep_pos + 1);
    } else {
      short_file_name = full_file_path;
    }

    std::string final_output =
        std::format("{:%Y-%m-%d %H:%M:%S} {}{}{} {}:{}: {}\n", time,
                    lvl_to_color_code(lvl), lvl_to_string(lvl), "\033[0m",
                    short_file_name, format.loc.line(), user_msg);

    {
      std::lock_guard lock(m_mutex);
      if (lvl == LogLevel::ERROR) {
        std::cerr << final_output;
      } else {
        std::cout << final_output;
      }
    }
  }

  constexpr std::string_view lvl_to_color_code(LogLevel lvl) {
    switch (lvl) {
      case LogLevel::DBG:
        return "\033[36m";
        break;
      case LogLevel::INFO:
        return "\033[32m";
        break;
      case LogLevel::WARN:
        return "\033[33m";
        break;
      case LogLevel::ERROR:
        return "\033[31m";
        break;
      default:
        return "\033[32m";
        break;
    }
  }

  constexpr std::string_view lvl_to_string(LogLevel lvl) {
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
        break;
    }
  }
};
