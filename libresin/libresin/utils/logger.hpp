#ifndef RESIN_UTIL_LOGGER_HPP
#define RESIN_UTIL_LOGGER_HPP

#include <array>
#include <chrono>
#include <cstdint>
#include <format>
#include <memory>
#include <mutex>
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

static constexpr std::array<std::string_view, 3> kLogPrefixes = {"ERROR", "WARN", "INFO"};
static constexpr std::string_view kDebugLogPrefix             = "DEBUG";
static constexpr uint32_t kMaxLogPrefixSize                   = 5;

inline std::string_view get_log_prefix(LogLevel level) { return kLogPrefixes[static_cast<uint32_t>(level) - 1]; }

class LoggerScribe {
 public:
  explicit LoggerScribe(LogLevel max_level);

  virtual ~LoggerScribe()                                                                    = default;
  virtual void vlog(std::string_view usr_fmt, std::format_args usr_args,
                    const std::chrono::time_point<std::chrono::system_clock>& time_point, std::string_view file_path,
                    const std::source_location& location, LogLevel level, bool is_debug_msg) = 0;

 protected:
  const LogLevel max_level_;
};

class TerminalLoggerScribe : public LoggerScribe {
 public:
  explicit TerminalLoggerScribe(LogLevel max_level = LogLevel::Info, bool use_stderr = false);

  void vlog(std::string_view usr_fmt, std::format_args usr_args,
            const std::chrono::time_point<std::chrono::system_clock>& time_point, std::string_view file_path,
            const std::source_location& location, LogLevel level, bool is_debug_msg) override;

 private:
  FILE* std_stream_;
};

// class FileLogger : public LoggerEntity {
//  public:
//   void vlog(std::string_view usr_fmt, std::format_args usr_args, const std::tm& date_time,
//             const std::source_location& location, LogLevel level, bool debug) override;
// };

class Logger {
 public:
  Logger();
  ~Logger();

  struct FormatWithLocation {
    const char* value;
    std::source_location loc;

    FormatWithLocation(const char* s, const std::source_location& l = std::source_location::current())  // NOLINT
        : value(s), loc(l) {}
  };

  template <typename... Args>
  static inline void err(FormatWithLocation fmt_loc, Args&&... args) {
    get_instance().log(LogLevel::Err, false, fmt_loc.loc, fmt_loc.value, args...);
  }

  template <typename... Args>
  static inline void warn(FormatWithLocation fmt_loc, Args&&... args) {
    get_instance().log(LogLevel::Warn, false, fmt_loc.loc, fmt_loc.value, args...);
  }

  template <typename... Args>
  static inline void info(FormatWithLocation fmt_loc, Args&&... args) {
    get_instance().log(LogLevel::Info, false, fmt_loc.loc, fmt_loc.value, args...);
  }

  template <typename... Args>
  static inline void debug(FormatWithLocation fmt_loc, Args&&... args) {
#ifndef NDEBUG
    get_instance().log(LogLevel::None, true, fmt_loc.loc, fmt_loc.value, args...);
#endif
  }

  template <typename... Args>
  void log(const LogLevel level, const bool debug, const std::source_location& location, const std::string_view fmt,
           Args&... args) {
    const std::lock_guard lock(mutex_);
    auto file_path = std::string_view(location.file_name() + file_name_start_pos_);

    for (const auto& scribe : scribes_) {
      scribe->vlog(fmt, std::make_format_args(args...), std::chrono::system_clock::now(), file_path, location, level,
                   debug);
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
  size_t file_name_start_pos_;
};

}  // namespace resin

#endif  // RESIN_HPP
