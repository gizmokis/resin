#include <libresin/utils/logger.hpp>
#include <memory>
#include <print>
#include <string_view>

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
#define IS_UNIX
#endif

#ifndef IS_UNIX
#include <windows.h>
#endif

namespace resin {

LoggerScribe::LoggerScribe(LogLevel max_level) : max_level_(max_level) {}

TerminalLoggerScribe::TerminalLoggerScribe(LogLevel max_level, bool use_stderr)
    : LoggerScribe(max_level), use_stderr_(use_stderr) {}

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

static void print_msg(std::string_view prefix, std::string_view usr_fmt, std::format_args usr_args,
                      const std::tm& date_time, const std::source_location& location, LogLevel level) {
  // Print logger message prefix with date
  std::print("[{0:{4}} {1:02}:{2:02}:{3:02}] ", prefix, date_time.tm_hour, date_time.tm_min, date_time.tm_sec,
             kMaxLogPrefixSize);

  // Print location
  if (level < LogLevel::Info) {
#ifdef NDEBUG
    std::print("`{0}`: ", location.function_name());
#else
    std::print("{0}({1}:{2}) `{3}`: ", location.file_name(), location.line(), location.column(),
               location.function_name());
#endif
  }

  // Print message provided by the user
  std::vprint_unicode(usr_fmt, usr_args);
}

void TerminalLoggerScribe::vlog(std::string_view usr_fmt, std::format_args usr_args, const std::tm& date_time,
                                const std::source_location& location, LogLevel level, bool is_debug_msg) {  // NOLINT
#ifndef NDEBUG
  if (is_debug_msg) {
#ifdef IS_UNIX
    begin_unix_terminal("[34m");  // Blue
#else
    WORD old_win_terminal_attributes = begin_win_terminal(FOREGROUND_BLUE);
#endif

    print_msg(kDebugLogPrefix, usr_fmt, usr_args, date_time, location, level);

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

  print_msg(get_log_prefix(level), usr_fmt, usr_args, date_time, location, level);

#ifdef IS_UNIX
  end_unix_terminal();
#else
  end_win_terminal(old_win_terminal_attributes);
#endif
}

Logger::~Logger() {}

void Logger::add_scribe(std::unique_ptr<LoggerScribe> scribe) { scribes_.push_back(std::move(scribe)); }

}  // namespace resin
