
#include <libresin/core/resources/sdf_shader_resource_parser.hpp>
#include <libresin/core/sdf_tree/sdf_primitive_type_manager.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>

namespace resin {

void SDFPrimitiveTypeManager::add_type(std::string&& name, std::string&& func_name, SDFParams&& params,
                                       std::string&& shader_content) noexcept {
  auto desc = SDFPrimitiveTypeDescription{.id                  = static_cast<uint32_t>(descs_.size()),
                                          .name                = std::move(name),
                                          .primitive_func_name = std::move(func_name),
                                          .params              = std::move(params),
                                          .shader_content      = std::move(shader_content)};
  descs_.push_back(desc);
}

void SDFPrimitiveTypeManager::add_type_from_shader_res(const ShaderResource& sh_res) {
  try {
    auto result = SDFShaderResourceParser::parse(sh_res);
    add_type(std::string(sh_res.get_name()), std::move(result.glsl_primitive_function_name), std::move(result.args),
             std::move(result.content));
  } catch (const std::exception&) {
    log_throw(ParserFailureException(std::format(
        "The SDF Primitive Type Manager is unable to parse the shader resource with name {}.", sh_res.get_name())));
  }
}

std::string SDFPrimitiveTypeManager::generate_sdfs_glsl_content() const noexcept {
  auto result = std::string();
  for (const auto& d : descs_) {
    result += d.shader_content;
  }

  return result;
}

}  // namespace resin
