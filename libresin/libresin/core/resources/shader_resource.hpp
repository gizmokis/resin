#ifndef RESIN_SHADER_RESOURCE_HPP
#define RESIN_SHADER_RESOURCE_HPP

#include <filesystem>
#include <libresin/core/resources/shader_type.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/string_views.hpp>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace resin {

class ShaderResourceManager;

/**
 * @brief To instantiate see `ShaderResourceManager`.
 *
 */
class ShaderResource {
 public:
  ShaderResource() = delete;

  const std::unordered_set<std::string>& get_external_definition_names() const;

  /**
   * @brief If shader resource contains a line #external_definition <ext_defi_name>, the following function will
   * replace it with #define <ext_defi_name> <defi_content>.
   *
   * @param ext_defi_name
   * @param defi_content
   * @return true if the external definition has been injected successfully
   * @return false if there is no external definition with the provided name
   */
  bool inject_external_definition(std::string_view ext_defi_name, std::string&& defi_content);

  /**
   * @brief Checks whether all external definitions has been injected.
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
  const std::string& raw_glsl() const { return raw_glsl_; }

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

  std::string_view name() const { return name_; }

  /**
   * @brief Returns shader type specific info about the shader.
   *
   * @return const ShaderType&
   */
  const ShaderType& type() const { return type_; }

  template <CShaderType TType>
  bool has_type() const {
    return std::holds_alternative<TType>(type_);
  }

  std::string_view type_extension() const {
    return std::visit([](const auto& t) { return t.extension(); }, type_);
  }

  std::string_view type_name() const {
    return std::visit([](const auto& t) { return t.name(); }, type_);
  }

 protected:
  friend ShaderResourceManager;

  explicit ShaderResource(std::string&& raw_glsl, std::string&& intermediate_glsl, std::string&& name,
                          ShaderType&& type, std::unordered_set<std::string>&& ext_defi_names,
                          std::optional<std::string>&& version);

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
  ShaderResource parse_res(std::string_view shader_content, ShaderType&& shader_type,
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
                                                   const ShaderType& sh_type, WordsStringViewIterator& it,
                                                   const WordsStringViewIterator& end, size_t curr_line);
  std::optional<std::string> process_name_macro(const std::optional<std::filesystem::path>& sh_path,
                                                const ShaderType& sh_type, WordsStringViewIterator& it,
                                                const WordsStringViewIterator& end, size_t curr_line);

  static void process_sdf_shader(ShaderType& sh_type, std::string& preprocessed_content, const std::string& name);

 private:
  std::vector<std::filesystem::path> visited_paths_;
  std::unordered_map<std::filesystem::path, std::shared_ptr<const ShaderResource>> cache_;

  static int shader_name_id_;
};

}  // namespace resin
#endif
