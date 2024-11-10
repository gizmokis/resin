#include <algorithm>
#include <libresin/utils/logger.hpp>
#include <memory>
#include <print>
#include <source_location>
#include <string_view>
#include <version/version.hpp>

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
#define IS_UNIX
#endif

#ifndef IS_UNIX
#include <windows.h>
#endif

namespace resin {

LoggerScribe::LoggerScribe(LogLevel max_level) : max_level_(max_level) {}

TerminalLoggerScribe::TerminalLoggerScribe(LogLevel max_level, bool use_stderr)
    : LoggerScribe(max_level), std_stream_(use_stderr ? stderr : stdout) {}

#ifdef IS_UNIX
static void begin_unix_terminal(std::string_view style) { std::print("\033{}", style); }
static void end_unix_terminal() { std::print("\033[0m\n"); }
#else
static WORD begin_win_terminal(WORD attributes) {
  HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO console_info;
  WORD old_win_terminal_attributes = 0;

  if (GetConsoleScreenBufferInfo(h_console, &console_info)) {
    old_win_terminal_attributes = console_info.wAttributes;
  }

  SetConsoleTextAttribute(h_console, attributes);
  return old_win_terminal_attributes;
}

static void end_win_terminal(WORD old_attributes) {
  HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(h_console, old_attributes);
  std::print("\n");
}
#endif

static void print_msg(FILE* stream, std::string_view prefix, std::string_view usr_fmt, std::format_args usr_args,
                      const std::chrono::time_point<std::chrono::system_clock>& time_point, std::string_view file_name,
                      const std::source_location& location, LogLevel level) {
  // Print logger message prefix with date
  auto local_time = std::chrono::zoned_time{std::chrono::current_zone(), time_point};
  std::print(stream, "[{0:{2}} {1:%H:%M:%OS}] ", prefix, local_time, kMaxLogPrefixSize);

  // Print location
  if (level < LogLevel::Info) {
#ifdef NDEBUG
    std::print(stream, "`{0}`: ", location.function_name());
#else
    std::print(stream, "{0}({1}:{2}) `{3}`: ", file_name, location.line(), location.column(), location.function_name());
#endif
  }

  // Print message provided by the user
  std::vprint_unicode(stream, usr_fmt, usr_args);
}

void TerminalLoggerScribe::vlog(std::string_view usr_fmt, std::format_args usr_args,
                                const std::chrono::time_point<std::chrono::system_clock>& time_point,
                                std::string_view file_path, const std::source_location& location, LogLevel level,
                                bool is_debug_msg) {  // NOLINT
#ifndef NDEBUG
  if (is_debug_msg) {
#ifdef IS_UNIX
    begin_unix_terminal("[34m");  // Blue
#else
    WORD old_win_terminal_attributes = begin_win_terminal(FOREGROUND_BLUE);
#endif

    print_msg(std_stream_, kDebugLogPrefix, usr_fmt, usr_args, time_point, file_path, location, level);

#ifdef IS_UNIX
    end_unix_terminal();
#else
    end_win_terminal(old_win_terminal_attributes);
#endif
    return;
  }
#endif

  if (level > max_level_) {
    return;
  }

#ifdef IS_UNIX
  if (level == LogLevel::Err) {
    begin_unix_terminal("[1;31m");  // Red bold
  } else if (level == LogLevel::Warn) {
    begin_unix_terminal("[1;33m");  // Yellow bold
  } else {
    begin_unix_terminal("[;37m");  // White
  }
#else
  WORD old_win_terminal_attributes = 0;
  if (level == LogLevel::Err) {
    old_win_terminal_attributes = begin_win_terminal(FOREGROUND_RED);
  } else if (level == LogLevel::Warn) {
    old_win_terminal_attributes = begin_win_terminal(FOREGROUND_RED | FOREGROUND_GREEN);
  } else {
    old_win_terminal_attributes = begin_win_terminal(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
  }
#endif

  print_msg(std_stream_, get_log_prefix(level), usr_fmt, usr_args, time_point, file_path, location, level);

#ifdef IS_UNIX
  end_unix_terminal();
#else
  end_win_terminal(old_win_terminal_attributes);
#endif
}

Logger::Logger() : file_name_start_pos_(0) {
  auto logger_path   = std::string{std::source_location::current().file_name()};
  auto proj_abs_path = std::string{RESIN_BUILD_ABS_PATH};

  // Ensure that common path convention is used
  std::replace(logger_path.begin() , logger_path.end(), '\\', '/');
  std::replace(proj_abs_path.begin() , proj_abs_path.end(), '\\', '/');

  if (logger_path.find(proj_abs_path, 0) == 0) {
    file_name_start_pos_ = proj_abs_path.size() + 1;
  }
}

Logger::~Logger() {}

void Logger::add_scribe(std::unique_ptr<LoggerScribe> scribe) { scribes_.push_back(std::move(scribe)); }

}  // namespace resin
