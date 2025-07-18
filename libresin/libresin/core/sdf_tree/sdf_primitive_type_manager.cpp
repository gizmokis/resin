
#include <cstdint>
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/core/resources/shader_type.hpp>
#include <libresin/core/sdf_tree/sdf_primitive_type_manager.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>
#include <ranges>

namespace resin {

uint32_t SDFPrimitiveTypeManager::add_type_from_shader_res(std::shared_ptr<const ShaderResource> sdf_shader_resource) {
  if (!sdf_shader_resource->has_type<SDFShaderType>()) {
    log_throw(
        UnsupportedShaderTypeException(std::format("Expected SDF Shader, but received shader with name {} of type {}.",
                                                   sdf_shader_resource->name(), sdf_shader_resource->type_name())));
  }

  std::string name = std::string(sdf_shader_resource->name());
  for (auto& d : descs_) {
    if (d.name == sdf_shader_resource->name()) {
      auto blank_filter = std::views::filter([](auto&& c) { return c != '\n' && c != '\r' && c != ' '; });
      if (std::ranges::equal(d.shader_res->raw_glsl() | blank_filter,  //
                             sdf_shader_resource->raw_glsl() | blank_filter)) {
        // the type already exists
        return d.id;
      }

      // the type has the name but different glsl code
      name += std::format("#{}", unique_id_++);
    }
  }

  const auto& sh_sdf_type = std::get<SDFShaderType>(sdf_shader_resource->type());

  auto id   = static_cast<uint32_t>(descs_.size());
  auto desc = SDFPrimitiveTypeDescription{
      .id         = id,
      .name       = std::move(name),
      .params     = sh_sdf_type.args,
      .shader_res = std::move(sdf_shader_resource)  //
  };

  descs_.push_back(desc);
  is_shader_dirty_ = true;

  Logger::info("Primitive type with name {} created", desc.name);

  return id;
}

uint32_t SDFPrimitiveTypeManager::add_type_from_shader_res(ShaderResource&& sdf_shader_resource) {
  return add_type_from_shader_res(std::make_shared<const ShaderResource>(std::move(sdf_shader_resource)));
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
