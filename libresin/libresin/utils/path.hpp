#ifndef RESIN_PATH_HPP
#define RESIN_PATH_HPP

#include <filesystem>
#include <string>

namespace resin {

std::filesystem::path get_executable_path();

std::filesystem::path get_executable_dir();

std::string path_to_utf8str(const std::filesystem::path& path);

std::filesystem::path utf8str_to_path(std::string_view str_path);

}  // namespace resin

#endif
