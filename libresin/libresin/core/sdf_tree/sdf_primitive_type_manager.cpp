
#include <cstdint>
#include <libresin/core/resources/shader_type.hpp>
#include <libresin/core/sdf_tree/sdf_primitive_type_manager.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>

namespace resin {

uint32_t SDFPrimitiveTypeManager::add_type_from_shader_res(std::shared_ptr<const ShaderResource> sh_res) {
  if (!sh_res->has_type<SDFShaderType>()) {
    log_throw(UnsupportedShaderTypeException(std::format(
        "Expected SDF Shader, but received shader with name {} of type {}.", sh_res->name(), sh_res->type_name())));
  }

  const auto& sh_sdf_type = std::get<SDFShaderType>(sh_res->type());

  auto id   = static_cast<uint32_t>(descs_.size());
  auto desc = SDFPrimitiveTypeDescription{
      .id         = id,
      .name       = std::string(sh_res->name()),
      .params     = sh_sdf_type.args,
      .shader_res = std::move(sh_res)  //
  };

  descs_.push_back(desc);

  is_shader_dirty_ = true;

  return id;
}

const std::string& SDFPrimitiveTypeManager::sdfs_glsl() {
  if (!is_shader_dirty_) {
    return sdfs_glsl_;
  }
  is_shader_dirty_ = false;

  sdfs_glsl_ = std::string();
  for (const auto& d : descs_) {
    if (auto content = d.shader_res->glsl()) {
      sdfs_glsl_ += *content;
    }
  }

  return sdfs_glsl_;
}

}  // namespace resin
