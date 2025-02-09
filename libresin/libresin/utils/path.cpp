#include <libresin/utils/path.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace resin {

std::filesystem::path get_executable_path() {
#if defined(_WIN32)
  wchar_t buffer[MAX_PATH];
  GetModuleFileNameW(nullptr, buffer, MAX_PATH);
  return std::filesystem::path(buffer);
#else
  return std::filesystem::read_symlink("/proc/self/exe");
#endif
}

std::filesystem::path get_executable_dir() { return get_executable_path().parent_path(); }

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
  const char* raw_path = str_path.data();
  int raw_length       = static_cast<int>(str_path.length());
  int size             = MultiByteToWideChar(CP_UTF8, 0, raw_path, raw_length, nullptr, 0);
  std::wstring wide_path(size, 0);
  MultiByteToWideChar(CP_UTF8, 0, raw_path, raw_length, &wide_path[0], size);
  return std::filesystem::path(wide_path);
#else
  return std::filesystem::path(str_path);
#endif
}

}  // namespace resin
