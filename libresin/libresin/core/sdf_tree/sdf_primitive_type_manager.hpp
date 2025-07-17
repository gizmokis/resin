#pragma once

#include <cstdint>
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/static_vector.hpp>

namespace resin {

using SDFParams = StaticVector<std::string, sdf_shader_consts::kSDFMaxParamCount>;

struct SDFPrimitiveTypeDescription {
  uint32_t id;
  std::string name;
  SDFParams params;

  /**
   * @brief SDF Shader type.
   *
   */
  std::shared_ptr<const ShaderResource> shader_res;
};

class SDFPrimitiveTypeManager {
 public:
  /**
   * @brief Adds a new primitive type based on the provided SDF (.sdf) shader resource. If there is a type with the same
   * code (without spaces, newlines and carriage returns), the the new type is created.
   *
   * @throws UnsupportedShaderTypeException Thrown when the shader is not of SDF type.
   *
   * @param sh_res
   */
  uint32_t add_type_from_shader_res(std::shared_ptr<const ShaderResource> sdf_shader_resource);

  /**
   * @brief Concatenates the SDFs shader contents of the primitive types in the manager and returns it. Useful
   * for injection into an arbitrary shader resource via external definitions.
   *
   * @return std::string
   */
  const std::string& sdfs_glsl();

  /**
   * @brief Returns the SDF Primitive Type by id.
   *
   * @throws OutOfRangeException Thrown when there is no Primitive Type with the provided id.
   *
   * @param id
   * @return const SDFPrimitiveTypeDescription&
   */
  const SDFPrimitiveTypeDescription& type_by_id(uint32_t id) const {
    if (id >= descs_.size()) {
      log_throw(OutOfRangeException("There is no Primitive Type with the provided id."));
    }
    return descs_[id];
  }

  bool is_id_valid(uint32_t id) const { return id < descs_.size(); }

  bool is_dirty() const { return is_shader_dirty_; }
  void mark_dirty() { is_shader_dirty_ = true; }

  auto begin() { return descs_.begin(); }
  auto end() { return descs_.end(); }

  auto begin() const { return descs_.begin(); }
  auto end() const { return descs_.end(); }

  void clear() { descs_.clear(); }

 private:
  std::vector<SDFPrimitiveTypeDescription> descs_;
  bool is_shader_dirty_ = true;
  uint32_t unique_id_   = 0;
  std::string sdfs_glsl_;
};

}  // namespace resin
