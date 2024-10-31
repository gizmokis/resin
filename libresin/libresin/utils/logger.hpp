#ifndef RESIN_UTIL_LOGGER_HPP
#define RESIN_UTIL_LOGGER_HPP

#include <format>
#include <memory>
#include <optional>
#include <print>
#include <source_location>
#include <string_view>
#include <vector>

namespace resin {

constexpr static std::array<std::string_view, 3> kLogPrefixes = {
    "ERROR",
    "WARNING",
    "INFO",
};

enum class LogLevel : uint32_t {
  None = 0,
  Err  = 1,
  Warn = 2,
  Info = 3,
};

inline constexpr std::string_view get_log_prefix(LogLevel level) {
  return kLogPrefixes[static_cast<uint32_t>(level) - 1];
}

class LoggerScribe {
 public:
  explicit LoggerScribe(LogLevel max_level);

  virtual ~LoggerScribe() = default;
  virtual void vlog(std::string_view usr_fmt, std::format_args usr_args, const std::tm& date_time,
                    const std::source_location& location, std::string_view location_file_name, LogLevel level) = 0;

 protected:
  const LogLevel max_level_;
};

class TerminalLoggerScribe : public LoggerScribe {
 public:
  explicit TerminalLoggerScribe(LogLevel max_level = LogLevel::Info, bool use_stderr = false);

  void vlog(std::string_view usr_fmt, std::format_args usr_args, const std::tm& date_time,
            const std::source_location& location, std::string_view location_file_name, LogLevel level) override;

 private:
  const bool use_stderr_;
};

// class FileLogger : public LoggerEntity {
//  public:
//   void vlog(std::string_view usr_fmt, std::format_args usr_args, const std::tm& date_time,
//             const std::source_location& location, LogLevel level) override;
// };

class Logger {
 public:
  // Initialize the logger. This function is not thread-safe and should be
  // called once at the beginning of the program execution. This
  // singleton class is NOT lazily evaluated -- any log attempt will not result
  // in any log message if this function is not invoked.
  static void init();

  ~Logger();

  struct FormatWithLocation {
    const char* value;
    std::source_location loc;

    FormatWithLocation(const char* s, const std::source_location& l = std::source_location::current())  // NOLINT
        : value(s), loc(l) {}
  };

  template <typename... Args>
  inline static void err(FormatWithLocation fmt_loc, Args&&... args) {
    log(LogLevel::Err, fmt_loc.loc, fmt_loc.value, args...);
  }

  template <typename... Args>
  inline static void warn(FormatWithLocation fmt_loc, Args&&... args) {
    log(LogLevel::Warn, fmt_loc.loc, fmt_loc.value, args...);
  }

  template <typename... Args>
  inline static void info(FormatWithLocation fmt_loc, Args&&... args) {
    log(LogLevel::Info, fmt_loc.loc, fmt_loc.value, args...);
  }

  template <typename... Args>
  static void log(LogLevel level, const std::source_location& location, std::string_view fmt, Args&&... args) {
    if (!s_instance_.has_value()) {
      return;
    }
    std::time_t t = std::time(nullptr);
    std::tm now   = *std::localtime(&t);

    auto full_file_name = std::string_view(location.file_name());

    size_t pos     = full_file_name.find("resin");
    auto file_name = full_file_name.substr(pos);

    // TODO: implement scribes
    // for (auto& scribe : scribes_) {
    //   scribe->vlog(fmt, std::make_format_args(args...), now, location, file_name, level);
    // }
  }

 private:
  static std::optional<Logger> s_instance_;
  static std::vector<std::unique_ptr<LoggerScribe>> scribes_;
};

}  // namespace resin

#endif  // RESIN_HPP
