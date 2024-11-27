#ifndef SHADER_RESOURCE_HPP
#define SHADER_RESOURCE_HPP

#include <array>
#include <cstdint>
#include <filesystem>
#include <libresin/core/resources/resource_manager.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/string_views.hpp>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace resin {

namespace shader_macros {

static constexpr std::string_view kIncludeMacro = "#include";
static constexpr std::string_view kExtDefiMacro = "#external_definition";
static constexpr std::string_view kVersionMacro = "#version";

static constexpr std::array<std::string_view, 3> kAllMacros = {
    kIncludeMacro,
    kExtDefiMacro,
    kVersionMacro,
};

inline bool is_macro(std::string_view word) { return std::ranges::find(kAllMacros, word) != kAllMacros.end(); }

}  // namespace shader_macros

enum class ShaderType : uint8_t {
  Vertex   = 0,
  Fragment = 1,
  Compute  = 2,
  Library  = 3,
};

static constexpr std::array<std::string_view, 4> kShaderTypeToExtensionMap = {".vert", ".frag", ".comp", ".glsl"};

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
  explicit ShaderResource(std::string&& content, ShaderType type, std::unordered_set<std::string>&& ext_defi_names,
                          std::optional<std::string>&& version);

  const std::unordered_set<std::string>& get_ext_defi_names() const;
  void set_ext_defi(std::string_view ext_defi_name, std::string&& defi_content);

  // Checks if all external definitions has been defined.
  bool is_glsl_ready() const;

  // Returns raw glsl shader with inserted dependencies.
  const std::string& get_raw() const;

  // Returns glsl shader with inserted dependencies, external defintions and version macro.
  const std::string& get_glsl() const;

  inline ShaderType get_type() const { return type_; }

  inline std::string_view get_extension() const { return kShaderTypeToExtensionMap[static_cast<uint8_t>(type_)]; }

 private:
  std::unordered_set<std::string> ext_defi_names_;
  std::unordered_map<std::string, std::string> ext_defi_contents_;

  std::optional<std::string> version_;
  std::string raw_content_;
  ShaderType type_;

  mutable bool is_dirty_;
  mutable std::string glsl_;
};

class ShaderResourceManager : public ResourceManager<ShaderResource> {
 public:
  ~ShaderResourceManager() override {}

 protected:
  ShaderResource load_res(const std::filesystem::path& path) override;

 private:
  template <ExceptionConcept Exception>
  [[noreturn]] void inline clear_log_throw(Exception&& e) {
    visited_paths_.clear();
    log_throw<Exception>(std::forward<Exception>(e));
  }

  void process_include_macro(const std::filesystem::path& sh_path, WordsStringViewIterator& it,
                             const WordsStringViewIterator& end, size_t curr_line, std::string& content,
                             std::unordered_set<std::string>& defi_names);
  void process_ext_defi_macro(const std::filesystem::path& sh_path, WordsStringViewIterator& it,
                              const WordsStringViewIterator& end, size_t curr_line,
                              std::unordered_set<std::string>& defi_names);
  std::optional<std::string> process_version_macro(const std::filesystem::path& sh_path, ShaderType sh_type,
                                                   WordsStringViewIterator& it, const WordsStringViewIterator& end,
                                                   size_t curr_line);

 private:
  std::vector<std::filesystem::path> visited_paths_;
};

}  // namespace resin
#endif
