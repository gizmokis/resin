#pragma once
#include <array>
#include <libresin/core/resources/shader_resource.hpp>
#include <string>

namespace resin {

class SDFShaderResourceParser {
 public:
  static constexpr auto kMaxSDFArguments = 3;

  struct Result {
    std::string glsl_primitive_function_name;
    std::string glsl_sdf_name;
    std::string content;
    std::array<std::optional<std::string>, kMaxSDFArguments> sdf_args;
    size_t sdf_args_count;
  };

  /**
   * @brief Expects a ShaderResource of type SDF (.sdf). Returns sdf and primitive functions with a proper name
   * and sdf function argument.
   *
   * @return SDFShaderResourceParser::Result
   */
  static Result parse(const ShaderResource& shader);
};

}  // namespace resin
