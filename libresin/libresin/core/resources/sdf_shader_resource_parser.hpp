#pragma once
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/utils/static_vector.hpp>
#include <string>

namespace resin {

class SDFShaderResourceParser {
 public:
  struct Result {
    std::string glsl_primitive_function_name;
    std::string glsl_sdf_name;

    /**
     * @brief Content without newlines.
     *
     */
    std::string content;

    StaticVector<std::string, sdf_shader_consts::kSDFMaxParamCount> args;
  };

  /**
   * @brief Expects a ShaderResource of type SDF (.sdf). Returns sdf and primitive functions with a proper name
   * and sdf function argument.
   *
   * @throw UnsupportedShaderTypeProvided Thrown when the shader resource is not of the SDF (.sdf) type.
   * @throw SDFShaderInvalidFunctionSignature Thrown when the sdf function signature is not found or appears more than
   * once or it's invalid (e.g. too many sdf function parameters).
   * @throw SDFShaderNoFunctionBodyFound Thrown when the sdf function signature was found but there is no function body.
   *
   * @return SDFShaderResourceParser::Result
   */
  static Result parse(const ShaderResource& shader);
};

}  // namespace resin
