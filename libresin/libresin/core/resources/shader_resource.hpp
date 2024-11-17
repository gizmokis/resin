#ifndef SHADER_RESOURCE_HPP
#define SHADER_RESOURCE_HPP

#include <array>
#include <cstdint>
#include <filesystem>
#include <libresin/core/resources/resource_manager.hpp>
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

class ShaderResourceManager;

class ShaderResource {
 public:
  ShaderResource() = delete;

  const std::unordered_set<std::string>& get_ext_defi_names() const;
  void set_ext_defi(std::string_view ext_defi_name, std::string&& defi_content);

  // Checks if all external definitions has been defined. Does not guarantee that the shader code is correct.
  bool is_glsl_ready() const;

  // Returns raw glsl shader with dependencies included, without any external definitions.
  const std::string& get_raw() const;

  // Returns glsl shader with dependencies included and with external defintions that were inserted.
  // Does not guarantee that the shader code is correct.
  const std::string& get_glsl() const;

 private:
  friend ShaderResourceManager;

  explicit ShaderResource(std::filesystem::path path, std::string&& content, ShaderType type,
                          std::unordered_set<std::string>&& ext_defi_names);

  std::filesystem::path path_;

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
  std::optional<ShaderResource> load_res(const std::filesystem::path& path) override;

 private:
  std::vector<std::filesystem::path> visited_paths_;
  int rec_level_;
};

}  // namespace resin
#endif
