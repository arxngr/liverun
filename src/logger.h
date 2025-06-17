#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace livrn {

enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

namespace Color {
constexpr const char *RESET = "\033[0m";
constexpr const char *RED = "\033[31m";
constexpr const char *GREEN = "\033[32m";
constexpr const char *YELLOW = "\033[33m";
constexpr const char *BLUE = "\033[34m";
} // namespace Color

class Logger {
public:
  template <typename... Args> static void debug(Args &&...args) {
    log(LogLevel::DEBUG, std::forward<Args>(args)...);
  }

  template <typename... Args> static void info(Args &&...args) {
    log(LogLevel::INFO, std::forward<Args>(args)...);
  }

  template <typename... Args> static void warn(Args &&...args) {
    log(LogLevel::WARNING, std::forward<Args>(args)...);
  }

  template <typename... Args> static void error(Args &&...args) {
    log(LogLevel::ERROR, std::forward<Args>(args)...);
  }

private:
  template <typename... Args> static void log(LogLevel level, Args &&...args) {
    std::ostringstream oss;
    (oss << ... << args); // Fold expression (C++17)

    const char *color = "";
    const char *label = "";

    switch (level) {
    case LogLevel::DEBUG:
      color = Color::GREEN;
      label = "DEBUG";
      break;
    case LogLevel::INFO:
      color = Color::BLUE;
      label = "INFO";
      break;
    case LogLevel::WARNING:
      color = Color::YELLOW;
      label = "WARN";
      break;
    case LogLevel::ERROR:
      color = Color::RED;
      label = "ERROR";
      break;
    }

    std::cout << color << "[livrn][" << label << "] " << currentTime() << " - "
              << oss.str() << Color::RESET << std::endl;
  }

  static std::string currentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    return oss.str();
  }
};

} // namespace livrn
