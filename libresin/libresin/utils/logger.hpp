#ifndef RESIN_UTIL_LOGGER_HPP
#define RESIN_UTIL_LOGGER_HPP

#include <cstdint>
#include <format>
#include <memory>
#include <print>
#include <source_location>
#include <string_view>
#include <vector>

namespace resin {

enum class LogLevel : uint32_t {
  None = 0,
  Err  = 1,
  Warn = 2,
  Info = 3,
};

constexpr static std::array<std::string_view, 3> kLogPrefixes = {"ERROR", "WARN", "INFO"};
constexpr static std::string_view kDebugLogPrefix             = "DEBUG";
constexpr static uint32_t kMaxLogPrefixSize                   = 5;

inline constexpr std::string_view get_log_prefix(LogLevel level) {
  return kLogPrefixes[static_cast<uint32_t>(level) - 1];
}

class LoggerScribe {
 public:
  explicit LoggerScribe(LogLevel max_level);

  virtual ~LoggerScribe()                                                             = default;
  virtual void vlog(std::string_view usr_fmt, std::format_args usr_args, const std::tm& date_time,
                    const std::source_location& location, LogLevel level, bool debug) = 0;

 protected:
  const LogLevel max_level_;
};

class TerminalLoggerScribe : public LoggerScribe {
 public:
  explicit TerminalLoggerScribe(LogLevel max_level = LogLevel::Info, bool use_stderr = false);

  void vlog(std::string_view usr_fmt, std::format_args usr_args, const std::tm& date_time,
            const std::source_location& location, LogLevel level, bool debug) override;

 private:
  const bool use_stderr_;
};

// class FileLogger : public LoggerEntity {
//  public:
//   void vlog(std::string_view usr_fmt, std::format_args usr_args, const std::tm& date_time,
//             const std::source_location& location, LogLevel level, bool debug) override;
// };

class Logger {
 public:
  Logger() = default;
  ~Logger();

  struct FormatWithLocation {
    const char* value;
    std::source_location loc;

    FormatWithLocation(const char* s, const std::source_location& l = std::source_location::current())  // NOLINT
        : value(s), loc(l) {}
  };

  template <typename... Args>
  inline static void err(FormatWithLocation fmt_loc, Args&&... args) {
    get_instance().log(LogLevel::Err, false, fmt_loc.loc, fmt_loc.value, args...);
  }

  template <typename... Args>
  inline static void warn(FormatWithLocation fmt_loc, Args&&... args) {
    get_instance().log(LogLevel::Warn, false, fmt_loc.loc, fmt_loc.value, args...);
  }

  template <typename... Args>
  inline static void info(FormatWithLocation fmt_loc, Args&&... args) {
    get_instance().log(LogLevel::Info, false, fmt_loc.loc, fmt_loc.value, args...);
  }

  template <typename... Args>
  inline static void debug(FormatWithLocation fmt_loc, Args&&... args) {
#ifndef NDEBUG
    get_instance().log(LogLevel::None, true, fmt_loc.loc, fmt_loc.value, args...);
#endif
  }

  template <typename... Args>
  void log(LogLevel level, bool debug, const std::source_location& location, std::string_view fmt, Args&&... args) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::time_t t = std::time(nullptr);
    std::tm now   = *std::localtime(&t);

    for (auto& scribe : scribes_) {
      scribe->vlog(fmt, std::make_format_args(args...), now, location, level, debug);
    }
  }

  void add_scribe(std::unique_ptr<LoggerScribe> scribe);

  static Logger& get_instance() {
    static Logger instance;
    return instance;
  }

 private:
  std::vector<std::unique_ptr<LoggerScribe>> scribes_;
  std::mutex mutex_;
};

}  // namespace resin

#endif  // RESIN_HPP
