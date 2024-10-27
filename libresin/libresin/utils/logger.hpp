#ifndef RESIN_UTIL_LOGGER_HPP
#define RESIN_UTIL_LOGGER_HPP

#include <filesystem>
#include <fstream>
#include <mutex>
#include <optional>
#include <print>
#include <source_location>

namespace resin {

class Logger {
 public:
  enum class LogLevel : uint32_t {
    None = 0,
    Err  = 1,
    Warn = 2,
    Info = 3,
  };

  static constexpr uint32_t kLogLevelsCount = 4;

  // Initialize the logger. This function is not thread-safe and should be
  // called once at the beginning of the program execution. This
  // singleton class is NOT lazily evaluated -- any log attempt will not result
  // in any log message if this function is not invoked.
  static void init(
      LogLevel max_level                                   = LogLevel::Warn,
      const std::optional<std::filesystem::path>& log_file = std::nullopt,
      LogLevel show_func_max_level                         = LogLevel::Warn,
      bool print_on_stdout                                 = true);

  ~Logger();

  inline static void err(
      std::string_view msg,
      const std::source_location& location = std::source_location::current()) {
    if (s_instance_.has_value()) {
      s_instance_->emit_msg(LogLevel::Err, msg, location);
    }
  }

  inline static void warn(
      std::string_view msg,
      const std::source_location& location = std::source_location::current()) {
    if (s_instance_.has_value()) {
      s_instance_->emit_msg(LogLevel::Warn, msg, location);
    }
  }

  inline static void info(
      std::string_view msg,
      const std::source_location& location = std::source_location::current()) {
    if (s_instance_.has_value()) {
      s_instance_->emit_msg(LogLevel::Info, msg, location);
    }
  }

  static void log(
      LogLevel level, std::string_view msg,
      const std::source_location& location = std::source_location::current()) {
    if (s_instance_.has_value()) {
      s_instance_->emit_msg(level, msg, location);
    }
  }

  Logger(LogLevel max_level,
         const std::optional<std::filesystem::path>& log_file,
         LogLevel show_func_max_level, bool print_on_stdout);

 private:
  void emit_msg(LogLevel level, std::string_view msg,
                const std::source_location& location);

  static std::optional<Logger> s_instance_;

  const LogLevel max_level_;
  const LogLevel show_func_max_level_;
  const bool print_on_stdout_;

  std::optional<std::ofstream> log_file_;

  std::mutex mutex_;
};

}  // namespace resin

#endif  // RESIN_HPP
