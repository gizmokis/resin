#pragma once

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
  std::string shader_content;
};

class SDFPrimitiveTypeManager {
 public:
  /**
   * @brief Parses the shader resource of type SDF (.sdf) and adds new primitive type based on that resource.
   *
   * @throws ParserFailureException Thrown when the parsing fails (e.g. no sdf function has been found or the resource
   * is not SDF resource).
   *
   * @param sh_res
   */
  void add_type_from_shader_res(const ShaderResource& sh_res);

  /**
   * @brief Concatenates the SDFs shader contents of the primitive types in the manager and returns it. Useful
   * for injection into an arbitrary shader resource via external definitions.
   *
   * @return std::string
   */
  std::string generate_sdfs_glsl_content() const noexcept;

  /**
   * @brief Returns the SDF Primitive Type by id.
   *
   * @throws OutOfRangeException Thrown when there is no Primitive Type with the provided id.
   *
   * @param id
   * @return const SDFPrimitiveTypeDescription&
   */
  const SDFPrimitiveTypeDescription& type_by_id(uint32_t id) {
    if (id >= descs_.size()) {
      log_throw(OutOfRangeException("There is no Primitive Type with the provided id."));
    }
    return descs_[id];
  }

  auto begin() { return descs_.begin(); }
  auto end() { return descs_.end(); }

  auto begin() const { return descs_.begin(); }
  auto end() const { return descs_.end(); }

 private:
  void add_type(std::string&& name, SDFParams&& params, std::string&& shader_content) noexcept;

 private:
  std::vector<SDFPrimitiveTypeDescription> descs_;
};

}  // namespace resin
