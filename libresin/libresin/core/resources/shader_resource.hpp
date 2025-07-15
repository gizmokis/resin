#ifndef RESIN_SHADER_RESOURCE_HPP
#define RESIN_SHADER_RESOURCE_HPP

#include <array>
#include <cstdint>
#include <filesystem>
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
static constexpr std::string_view kNameMacro    = "#name";

static constexpr std::array<std::string_view, 4> kAllMacros = {
    kIncludeMacro,  //
    kExtDefiMacro,  //
    kVersionMacro,  //
    kNameMacro      //
};

inline bool is_macro(std::string_view word) { return std::ranges::find(kAllMacros, word) != kAllMacros.end(); }

}  // namespace shader_macros

enum class ShaderType : uint8_t {
  Vertex   = 0,
  Fragment = 1,
  Compute  = 2,
  Library  = 3,
  SDF      = 4,
  _Count   = 5,  // NOLINT
};

static constexpr std::array<std::string_view, static_cast<size_t>(ShaderType::_Count)> kShaderTypeToExtensionMap = {
    ".vert", ".frag", ".comp", ".glsl", ".sdf"};

inline std::optional<ShaderType> extension_to_shader_type(std::string_view extension) {
  for (size_t i = 0; i < kShaderTypeToExtensionMap.size(); ++i) {
    if (kShaderTypeToExtensionMap[i] == extension) {
      return static_cast<ShaderType>(i);
    }
  }
  return std::nullopt;
}

class ShaderResourceManager;

/**
 * @brief To instantiate see `ShaderResourceManager`.
 *
 */
class ShaderResource {
 public:
  ShaderResource() = delete;

  const std::unordered_set<std::string>& get_ext_defi_names() const;
  void set_ext_defi(std::string_view ext_defi_name, std::string&& defi_content);

  /**
   * @brief Checks whether all external definitions has been resolved.
   *
   * @return true
   * @return false
   */
  bool is_glsl_ready() const;

  /**
   * @brief Returns raw GLSL content before parsing.
   *
   * @return const std::string&
   */
  const std::string& raw_glsl() const;

  /**
   * @brief Returns intermediate GLSL shader string. External definitions are not injected. The compilation process
   * might result in errors.
   *
   * @return const std::string&
   */
  const std::string& intermediate_glsl() const { return intermediate_glsl_; }

  /**
   * @brief Returns GLSL shader if all external definitions are resolved, std::nullopt otherwise.
   *
   * @return std::optional<std::string_view>
   */
  std::optional<std::string_view> glsl() const;

  ShaderType get_type() const { return type_; }
  std::string_view get_extension() const { return kShaderTypeToExtensionMap[static_cast<uint8_t>(type_)]; }

  std::string_view get_name() const { return name_; }

 protected:
  friend ShaderResourceManager;

  explicit ShaderResource(std::string&& raw_glsl, std::string&& intermediate_glsl, std::string&& name, ShaderType type,
                          std::unordered_set<std::string>&& ext_defi_names, std::optional<std::string>&& version);

 private:
  std::unordered_set<std::string> ext_defi_names_;
  std::unordered_map<std::string, std::string> ext_defi_contents_;

  std::optional<std::string> version_;
  std::string name_;
  ShaderType type_;

  std::string raw_glsl_;
  std::string intermediate_glsl_;
  mutable std::string glsl_;
  mutable bool is_dirty_;
};

class ShaderResourceManager {
 public:
  /**
   * @brief Parses and caches the shader resource under the provided path.
   *
   * @param path
   * @return const std::shared_ptr<const ShaderResource>&
   */
  const std::shared_ptr<const ShaderResource>& get_res_ptr(const std::filesystem::path& path);

  /**
   * @brief Parses and caches the shader resource under the provided path.
   *
   * @param path
   * @return const std::shared_ptr<const ShaderResource>&
   */
  const ShaderResource& get_res(const std::filesystem::path& path) { return *get_res_ptr(path); }

  /**
   * @brief Parses the shader resource without caching.
   *
   * @param path
   * @return ShaderResource
   */
  ShaderResource parse_res(const std::filesystem::path& path);

  /**
   * @brief Parses the shader resource without caching. Allows to parse shader resource without their path.
   *
   * @param shader_content The content of a shader resource to be parsed.
   * @param shader_type The shader type of the content, e.g. fragment shader.
   * @param path The direct path to the shader or current working directory. It's required when #include macro is used.
   * @return ShaderResource
   */
  ShaderResource parse_res(std::string_view shader_content, ShaderType shader_type,
                           std::optional<std::filesystem::path> path = std::nullopt);

 private:
  template <ExceptionConcept Exception>
  [[noreturn]] void clear_log_throw(Exception&& e) {
    visited_paths_.clear();
    log_throw<Exception>(std::forward<Exception>(e));
  }

  void process_include_macro(const std::filesystem::path& sh_path, WordsStringViewIterator& it,
                             const WordsStringViewIterator& end, size_t curr_line, std::string& content,
                             std::unordered_set<std::string>& defi_names);
  void process_ext_defi_macro(const std::optional<std::filesystem::path>& sh_path, WordsStringViewIterator& it,
                              const WordsStringViewIterator& end, size_t curr_line,
                              std::unordered_set<std::string>& defi_names);
  std::optional<std::string> process_version_macro(const std::optional<std::filesystem::path>& sh_path,
                                                   ShaderType sh_type, WordsStringViewIterator& it,
                                                   const WordsStringViewIterator& end, size_t curr_line);
  std::optional<std::string> process_name_macro(const std::optional<std::filesystem::path>& sh_path, ShaderType sh_type,
                                                WordsStringViewIterator& it, const WordsStringViewIterator& end,
                                                size_t curr_line);

 private:
  std::vector<std::filesystem::path> visited_paths_;
  std::unordered_map<std::filesystem::path, std::shared_ptr<const ShaderResource>> cache_;

  static int shader_name_id_;
};

}  // namespace resin
#endif
