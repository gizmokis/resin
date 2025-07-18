#pragma once

#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/utils/macros.hpp>
#include <libresin/utils/static_vector.hpp>
#include <string>
#include <variant>

namespace resin {

namespace shader_macros {

static constexpr std::string_view kSingleLineCommentRegExpStr = R"(\/\/.*\r?\n)";

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

template <typename T>
concept CShaderType = requires(std::string_view macro) {
  { T::name() } -> std::convertible_to<std::string_view>;
  { T::extension() } -> std::convertible_to<std::string_view>;
  { T::is_macro_supported(macro) } -> std::same_as<bool>;
};

struct VertexShaderType {
  [[nodiscard]] static constexpr std::string_view extension() { return ".vert"; }
  [[nodiscard]] static constexpr std::string_view name() { return "Vertex Shader"; }
  [[nodiscard]] static bool is_macro_supported(std::string_view macro) { return shader_macros::is_macro(macro); }
};

struct FragmentShaderType {
  [[nodiscard]] static constexpr std::string_view extension() { return ".frag"; }
  [[nodiscard]] static constexpr std::string_view name() { return "Fragment Shader"; }
  [[nodiscard]] static bool is_macro_supported(std::string_view macro) { return shader_macros::is_macro(macro); }
};

struct ComputeShaderType {
  [[nodiscard]] static constexpr std::string_view extension() { return ".comp"; }
  [[nodiscard]] static constexpr std::string_view name() { return "Compute Shader"; }
  [[nodiscard]] static bool is_macro_supported(std::string_view macro) { return shader_macros::is_macro(macro); }
};

struct LibraryShaderType {
  [[nodiscard]] static constexpr std::string_view extension() { return ".glsl"; }
  [[nodiscard]] static constexpr std::string_view name() { return "Library Shader"; }
  [[nodiscard]] static bool is_macro_supported(std::string_view macro) { return shader_macros::is_macro(macro); }
};

struct SDFShaderType {
  [[nodiscard]] static constexpr std::string_view extension() { return ".sdf"; }
  [[nodiscard]] static constexpr std::string_view name() { return "SDF Shader"; }
  [[nodiscard]] static bool is_macro_supported(std::string_view macro) { return macro == shader_macros::kNameMacro; }

  /**
   * @brief The generated SDF function name.
   *
   */
  std::string glsl_sdf_name;

  /**
   * @brief Parsed argument names.
   *
   */
  StaticVector<std::string, sdf_shader_consts::kSDFMaxParamCount> args;
};

using ShaderType =
    std::variant<VertexShaderType, FragmentShaderType, ComputeShaderType, LibraryShaderType, SDFShaderType>;
RESIN_VALIDATE_VARIANT_TYPES(ShaderType, CShaderType);

constexpr std::optional<ShaderType> extension_to_shader_type(std::string_view extension) {
  if (extension == VertexShaderType::extension()) {
    return VertexShaderType{};
  }
  if (extension == FragmentShaderType::extension()) {
    return FragmentShaderType{};
  }
  if (extension == ComputeShaderType::extension()) {
    return ComputeShaderType{};
  }
  if (extension == LibraryShaderType::extension()) {
    return LibraryShaderType{};
  }
  if (extension == SDFShaderType::extension()) {
    return SDFShaderType{};
  }
  return std::nullopt;
}

}  // namespace resin
