#ifndef RESIN_GLSL_NAMES_HPP
#define RESIN_GLSL_NAMES_HPP
#include <cstdint>
#include <libresin/utils/enum_mapper.hpp>

namespace resin {

namespace sdf_shader_consts {

enum class SDFShaderBinOp : uint8_t {
  Union       = 0,
  SmoothUnion = 1,
  Diff        = 2,
  SmoothDiff  = 3,
  Inter       = 4,
  SmoothInter = 5,
  Xor         = 6,
  SmoothXor   = 7,
  _Count      = 8  // NOLINT
};
constexpr StringEnumMapping<SDFShaderBinOp> kSDFShaderBinOpFunctionNames({"opUnion", "opSmoothUnion", "opDiff",
                                                                          "opSmoothDiff", "opInter", "opSmoothInter",
                                                                          "opXor", "opSmoothXor"});

enum class SDFShaderPrim : uint8_t {
  Sphere = 0,
  Cube   = 1,
  _Count = 2,  // NOLINT
};
constexpr StringEnumMapping<SDFShaderPrim> kSDFShaderPrimFunctionNames({"sdSphere", "sdCube"});
constexpr StringEnumMapping<SDFShaderPrim> kSDFShaderPrimComponentArrayNames({
    "u_spheres",
    "u_cubes",
});

constexpr std::string_view kSDFPrimitivesArrayName = "u_sdf_primitives";

enum class SDFShaderCoreComponents : uint8_t {
  Transforms = 0,
  Materials  = 1,
  _Count     = 2,  // NOLINT
};
constexpr StringEnumMapping<SDFShaderCoreComponents> kSDFShaderCoreComponentArrayNames({
    "u_transforms",
    "u_materials",
});

enum class SDFShaderVariable : uint8_t {
  Position = 0,
  _Count   = 1,  // NOLINT
};
constexpr StringEnumMapping<SDFShaderVariable> kSDFShaderVariableNames({"pos"});

}  // namespace sdf_shader_consts

}  // namespace resin

#endif
