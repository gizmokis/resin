#ifndef SHADER_RESOURCE_HPP
#define SHADER_RESOURCE_HPP

#include <array>
#include <cstdint>
#include <filesystem>
#include <libresin/core/resources/resource_manager.hpp>
#include <string_view>
#include <vector>

namespace resin {

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

  static std::optional<ShaderResource> create(std::string&& content, ShaderType type);
  static std::optional<ShaderResource> from_path(const std::filesystem::path& path);

  using ExternalDefinitionsIterator = std::vector<std::string>::const_iterator;
  using DependenciesIterator        = std::vector<std::filesystem::path>::const_iterator;

  std::pair<ExternalDefinitionsIterator, ExternalDefinitionsIterator> get_ext_defi_names() const {
    return {ext_defi_names_.cbegin(), ext_defi_names_.cend()};
  }

  std::pair<DependenciesIterator, DependenciesIterator> get_deps_rel_paths() const {
    return {deps_rel_paths_.cbegin(), deps_rel_paths_.cend()};
  }

  // TODO
  // void insert_dep(const std::filesystem::path& dep_path, std::string_view dep_content);
  // void set_ext_defi(std::string_view ext_defi_name, std::string_view defi_content);
  // bool is_glsl_ready() const;
  // const std::string& get_str() const;

 private:
  explicit ShaderResource(std::string&& content, ShaderType type, std::vector<std::filesystem::path>&& deps_rel_paths,
                          std::vector<std::string>&& ext_defi_names);

 private:
  std::vector<std::filesystem::path> deps_rel_paths_;
  std::vector<std::string> ext_defi_names_;

  std::string content_;
  ShaderType type_;
};

class ShaderResourceManager : ResourceManager<ShaderResource> {
  std::optional<ShaderResource> load_res(const std::filesystem::path& path) override;
};

}  // namespace resin
#endif
