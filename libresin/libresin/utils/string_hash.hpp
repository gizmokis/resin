#ifndef RESIN_STRING_HASH_HPP
#define RESIN_STRING_HASH_HPP

#include <string>
#include <string_view>

namespace resin {

struct StringHash {
  using is_transparent = void;

  [[nodiscard]] size_t operator()(const char* txt) const { return std::hash<std::string_view>{}(txt); }
  [[nodiscard]] size_t operator()(std::string_view txt) const { return std::hash<std::string_view>{}(txt); }
  [[nodiscard]] size_t operator()(const std::string& txt) const { return std::hash<std::string>{}(txt); }
};

}  // namespace resin

#endif  // RESIN_STRING_HASH_HPP