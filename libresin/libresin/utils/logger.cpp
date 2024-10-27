#include <iostream>
#include <libresin/utils/logger.hpp>
#include <optional>
#include <print>

namespace resin {

std::optional<Logger> Logger::s_instance_ = std::nullopt;

void Logger::init(LogLevel max_level,
                  const std::optional<std::filesystem::path>& log_file,
                  LogLevel show_func_max_level, bool print_on_stdout) {
  if (s_instance_.has_value()) {
    Logger::err("Logger must not be initialized more than once.");
    std::abort();
  }

  s_instance_.emplace(max_level, log_file, show_func_max_level,
                      print_on_stdout);
}

Logger::Logger(LogLevel max_level,
               const std::optional<std::filesystem::path>& log_file,
               LogLevel show_func_max_level, bool print_on_stdout)
    : max_level_(max_level),
      log_file_(std::nullopt),
      show_func_max_level_(show_func_max_level),
      print_on_stdout_(print_on_stdout) {
  if (log_file.has_value()) {
    log_file_.emplace(log_file.value(), std::ios::ate);

    if (!log_file_.value()) {
      std::println(std::cerr, "Could not open the log file {0}",
                   log_file.value().c_str());
      log_file_ = std::nullopt;
    }
  }
}

Logger::~Logger() {}

void Logger::emit_msg(LogLevel level, std::string_view msg,
                      const std::source_location& location) {
  if (level > max_level_ || max_level_ == LogLevel::None ||
      level == LogLevel::None) {
    return;
  }

  if (!print_on_stdout_ && !log_file_.has_value()) {
    return;
  }

  std::lock_guard<std::mutex> lock(mutex_);

  static std::array<const char*, 3> s_prefixes = {
      "ERROR",
      "WARNING",
      "INFO",
  };

  std ::time_t epoch = std::time(nullptr);
  std::tm* lt        = std::localtime(&epoch);

  auto log_message = [&](std::ostream& stream) {
    std::print(stream, "[{0} {1}:{2}:{3}]",
               s_prefixes[static_cast<uint32_t>(level) - 1], lt->tm_hour,
               lt->tm_min, lt->tm_sec);

    if (level <= show_func_max_level_) {
      std::print(stream, " {0}({1}:{2}) `{3}`", location.file_name(),
                 location.line(), location.column(), location.function_name(),
                 msg);
    }

    std::print(stream, ": {0}\n", msg);
  };

  if (print_on_stdout_) {
    log_message(std::cout);
  }

  if (log_file_.has_value()) {
    log_message(log_file_.value());
  }
}

}  // namespace resin
