#include <libresin/utils/logger.hpp>
#include <optional>
#include <print>

namespace resin {

std::optional<Logger> Logger::s_instance_ = std::nullopt;

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
                                const std::source_location& location, std::string_view location_file_name,
                                LogLevel level) {
  if (level > max_level_) {
    return;
  }

  std::print("[{0} {1}:{2}:{3}] ", get_log_prefix(level), date_time.tm_hour, date_time.tm_min, date_time.tm_sec);

  if (level < LogLevel::Info) {
    std::print("{0}({1}:{2}) `{3}`\n", location.file_name(), location.line(), location.column(),
               location.function_name());
  }

  vprint(usr_fmt, usr_args);
  std::print("\n");
}

void Logger::init() {
  if (s_instance_.has_value()) {
    std::print(stderr, "Logger must not be initialized more than once!");
    std::abort();
  }

  s_instance_.emplace();
}

Logger::~Logger() {}

}  // namespace resin
