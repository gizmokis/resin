#include <libresin/utils/logger.hpp>
#include <memory>
#include <print>

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
#define IS_UNIX
#endif

namespace resin {

// Adapted from the GCC `std::print` implementation that may be found here:
// https://github.com/gcc-mirror/gcc/blob/7e1d9f58858153bee4bcbab45aa862442859d958/libstdc%2B%2B-v3/include/std/print#L104C1-L113C6
// The vprint differs from the std::print implementation in that it's not being generic variadic function, which enables
// its use it in `LoggerScribe`s. Sadly I haven't found the better workaround.
static void vprint(FILE* stream, std::string_view fmt, std::format_args args) {
  if constexpr (std::__unicode::__literal_encoding_is_utf8()) {
    std::vprint_unicode(stream, fmt, args);
  } else {
    std::vprint_nonunicode(stream, fmt, args);
  }
}

static void vprint(std::string_view fmt, std::format_args args) { vprint(stdout, fmt, args); }

LoggerScribe::LoggerScribe(LogLevel max_level) : max_level_(max_level) {}

TerminalLoggerScribe::TerminalLoggerScribe(LogLevel max_level, bool use_stderr)
    : LoggerScribe(max_level), use_stderr_(use_stderr) {}

void TerminalLoggerScribe::vlog(std::string_view usr_fmt, std::format_args usr_args, const std::tm& date_time,
                                const std::source_location& location, LogLevel level, bool debug) {
  if (!debug && level > max_level_) {
    return;
  }

#ifdef IS_UNIX
  if (level == LogLevel::Err) {
    std::print("\033[1;31m");  // Red bold
  } else if (level == LogLevel::Warn) {
    std::print("\033[1;33m");  // Yellow bold
  } else if (debug) {
    std::print("\033[34m");  // Blue
  }
#endif

  std::print("[{0:{4}} {1:02}:{2:02}:{3:02}] ", debug ? kDebugLogPrefix : get_log_prefix(level), date_time.tm_hour,
             date_time.tm_min, date_time.tm_sec, kMaxLogPrefixSize);

  if (level < LogLevel::Info) {
    std::print("{0}({1}:{2}) `{3}`: ", location.file_name(), location.line(), location.column(),
               location.function_name());
  }

  vprint(usr_fmt, usr_args);

#ifdef IS_UNIX
  std::print("\033[0m\n");  // Restore default text style
#endif
}

Logger::~Logger() {}

void Logger::add_scribe(std::unique_ptr<LoggerScribe> scribe) { scribes_.push_back(std::move(scribe)); }

}  // namespace resin
