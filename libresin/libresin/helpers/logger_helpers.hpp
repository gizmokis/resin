#ifndef LOGGER_HELPERS_HPP
#define LOGGER_HELPERS_HPP
#include <libresin/utils/logger.hpp>

namespace resin {

namespace helpers {

template <typename... Args>
inline void log_sh_err(int64_t line, std::string_view fmt, Args&&... args) {
  resin::Logger::err("Shader resource error (line {}): {}", line, std::vformat(fmt, std::make_format_args(args...)));
}

template <typename... Args>
inline void log_file_err(const std::filesystem::path& path, std::string_view fmt, Args&&... args) {
  resin::Logger::err("Could not load file with path \"{}\": {}", path.string(),
                     std::vformat(fmt, std::make_format_args(args...)));
}

}  // namespace helpers

}  // namespace resin

#endif  // LOGGER_HELPERS_HPP
