#ifndef RESIN_PATH_UTF_HPP
#define RESIN_PATH_UTF_HPP

#include <filesystem>
#include <string>

namespace resin {

std::string path_to_utf8str(const std::filesystem::path& path);

std::filesystem::path utf8str_to_path(std::string_view str_path);

}  // namespace resin

#endif
