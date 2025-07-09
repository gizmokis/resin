#include <algorithm>
#include <libresin/core/resources/sdf_shader_resource_parser.hpp>
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>
#include <libresin/utils/static_vector.hpp>
#include <regex>

namespace resin {

SDFShaderResourceParser::Result SDFShaderResourceParser::parse(const ShaderResource& shader) {
  if (shader.get_type() != ShaderType::SDF) {
    log_throw(UnsupportedShaderTypeProvided("SDFShaderResourceParser expects shader of SDF type"));
  }

  const auto& sh_content = shader.get_glsl();

  static auto sdf_signature_pattern = std::regex(
      R"(float[\s\r\n]*sdf[\s\r\n]*\([\s\r\n]*vec3[\s\r\n]*\w+[\s\r\n]*(\)|(,[\s\r\n]*float[\s\r\n]*\w+[\s\r\n]*){1,3}\))+[\s\r\n]*)");
  static auto sdf_signature_with_body_pattern = std::regex(
      R"(float[\s\r\n]*sdf[\s\r\n]*\([\s\r\n]*vec3[\s\r\n]*\w+[\s\r\n]*(\)|(,[\s\r\n]*float[\s\r\n]*\w+[\s\r\n]*){1,3}\))+[\s\r\n]*\{([\s\r\n]|.)*\})");
  static auto sdf_argument_pattern = std::regex(R"(,[\s\r\n]*float[\s\r\n]*(\w+))");
  static auto sdf_name_pattern     = std::regex(R"(float[\s\r\n]*(sdf)[\s\r\n]*\()");

  auto iter      = std::sregex_iterator(sh_content.begin(), sh_content.end(), sdf_signature_pattern);
  const auto end = std::sregex_iterator();

  if (iter == end) {
    log_throw(SDFShaderInvalidFunctionSignature(std::string(shader.get_name())));
  }
  auto sdf_signature_match = *iter;
  if (++iter != end) {
    log_throw(SDFShaderInvalidFunctionSignature(std::string(shader.get_name())));
  }

  iter = std::sregex_iterator(sh_content.begin() + sdf_signature_match.position(), sh_content.end(),
                              sdf_signature_with_body_pattern);
  if (iter == end) {
    log_throw(SDFShaderNoFunctionBodyFound(std::string(shader.get_name())));
  }

  auto sdf_with_body = *iter;

  iter = std::sregex_iterator(sh_content.begin() + sdf_signature_match.position(),
                              sh_content.begin() + sdf_signature_match.position() + sdf_signature_match.length(),
                              sdf_argument_pattern);

  auto sdf_args_count = 0U;

  StaticVector<std::string, sdf_shader_consts::kSDFMaxParamCount> sdf_args;
  for (; iter != end; ++iter, ++sdf_args_count) {
    sdf_args.emplace(iter->str(1));
  }

  iter = std::sregex_iterator(sh_content.begin() + sdf_signature_match.position(),
                              sh_content.begin() + sdf_signature_match.position() + sdf_signature_match.length(),
                              sdf_name_pattern);
  if (iter == end) {
    log_throw(SDFShaderNoFunctionBodyFound(std::string(shader.get_name())));
  }
  auto sdf_name = *iter;

  auto glsl_sdf_name = std::format("{}_SDF", shader.get_name());

  auto sdf_with_body_start = static_cast<size_t>(sdf_name.position() + sdf_name.length());
  auto sdf_with_body_end   = static_cast<size_t>(sdf_name.position() + sdf_name.length() + sdf_with_body.length());
  auto sdf_func =
      std::format("float {}({}", glsl_sdf_name, sdf_with_body.str().substr(sdf_with_body_start, sdf_with_body_end));
  auto glsl_primitive_function_name = std::format("{}_Primitive", shader.get_name());

  auto args = std::string();
  if (sdf_args_count == 1) {
    args = ",u_sdf_primitives[primitive_id].size.x";
  } else if (sdf_args_count == 2) {
    args = ",u_sdf_primitives[primitive_id].size.x,u_sdf_primitives[primitive_id].size.y";
  } else {
    args =
        ",u_sdf_primitives[primitive_id].size.x,u_sdf_primitives[primitive_id].size.y,u_sdf_primitives[primitive_id]."
        "size.z";
  }

  auto primitive_func = std::format(R"(
  sdf_result {}(vec3 pos, int node_id, int primitive_id) {{
    sdf_result res;
    prepare(res, pos, node_id, primitive_id);
    res.dist = {}(pos{});
    return opScale(res, node_id);
  }})",
                                    glsl_primitive_function_name, glsl_sdf_name, args);

  auto sdf_content = sdf_func + primitive_func;

  auto r = std::ranges::remove_if(sdf_content, [](auto&& c) { return c == '\n' or c == '\r'; });
  sdf_content.erase(r.begin(), r.end());

  return Result{.glsl_primitive_function_name = glsl_primitive_function_name,
                .glsl_sdf_name                = glsl_sdf_name,
                .content                      = std::move(sdf_content),
                .args                         = std::move(sdf_args)};
}

}  // namespace resin
