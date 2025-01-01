#include <libresin/utils/path_utf.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace resin {

std::string path_to_utf8str(const std::filesystem::path& path) {
#ifdef _WIN32
  std::wstring wide_path = path.wstring();
  int size               = WideCharToMultiByte(CP_UTF8, 0, wide_path.c_str(), -1, nullptr, 0, nullptr, nullptr);
  std::string utf8_path(size, 0);
  WideCharToMultiByte(CP_UTF8, 0, wide_path.c_str(), -1, &utf8_path[0], size, nullptr, nullptr);
  return utf8_path;
#else
  return path.string();
#endif
}

std::filesystem::path utf8str_to_path(std::string_view str_path) {
#ifdef _WIN32
  int size = MultiByteToWideChar(CP_UTF8, 0, str_path.c_str(), -1, nullptr, 0);
  std::wstring wide_path(size, 0);
  MultiByteToWideChar(CP_UTF8, 0, str_path.c_str(), -1, &wide_path[0], size);
  return std::filesystem::path(wide_path);
#else
  return std::filesystem::path(str_path);
#endif
}

}  // namespace resin
