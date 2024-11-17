#ifndef SHADER_RESOURCE_HPP
#define SHADER_RESOURCE_HPP

#include <array>
#include <cstdint>
#include <filesystem>
#include <libresin/core/resources/resource_manager.hpp>
#include <libresin/utils/exceptions.hpp>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace resin {

namespace shader_macros {
static constexpr const char* kSupportedShaderVersion = "#version 150\n";

static constexpr const char* kIncludeMacro = "#include";
static constexpr const char* kExtDefiMacro = "#external_definition";
static constexpr const char* kVersionMacro = "#version";

static constexpr std::array<const char*, 3> kAllMacros = {
    kIncludeMacro,
    kExtDefiMacro,
    kVersionMacro,
};

inline bool is_macro(std::string_view word) { return std::ranges::find(kAllMacros, word) != kAllMacros.end(); }
}  // namespace shader_macros

enum class ShaderType : uint32_t {
  Vertex   = 0,
  Fragment = 1,
  Library  = 2,
};

static constexpr std::array<std::string_view, 3> kShaderTypeToExtensionMap = {".vert", ".frag", ".glsl"};

inline std::optional<ShaderType> extension_to_shader_type(std::string_view extension) {
  for (size_t i = 0; i < kShaderTypeToExtensionMap.size(); ++i) {
    if (kShaderTypeToExtensionMap[i] == extension) {
      return static_cast<ShaderType>(i);
    }
  }
  return std::nullopt;
}

class ShaderResource {
 public:
  ShaderResource() = delete;

  explicit ShaderResource(std::string&& content, ShaderType type, std::unordered_set<std::string>&& ext_defi_names);

  const std::unordered_set<std::string>& get_ext_defi_names() const;
  void set_ext_defi(std::string_view ext_defi_name, std::string&& defi_content);

  // Checks if all external definitions has been defined.
  bool is_glsl_ready() const;

  // Returns raw glsl shader with dependencies included, without any external definitions.
  const std::string& get_raw() const;

  // Returns glsl shader with dependencies included and with external defintions that were inserted.
  const std::string& get_glsl() const;

  inline ShaderType get_type() const { return type_; }

 private:
  std::unordered_set<std::string> ext_defi_names_;
  std::unordered_map<std::string, std::string> ext_defi_contents_;

  std::string raw_content_;
  ShaderType type_;

  mutable bool is_dirty_;
  mutable std::string glsl_;
};

class ShaderResourceManager : public ResourceManager<ShaderResource> {
 public:
  ~ShaderResourceManager() override {}
  ShaderResource load_res(const std::filesystem::path& path) override;

 private:
  template <ResinExceptionConcept Exception>
  inline void clear_and_throw() {
    visited_paths_.clear();
    throw Exception();
  }
  std::vector<std::filesystem::path> visited_paths_;
};

}  // namespace resin
#endif
