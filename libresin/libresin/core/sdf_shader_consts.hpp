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
constexpr EnumMapping<SDFShaderBinOp> kSDFShaderBinOpFunctionNames({"opUnion", "opSmoothUnion", "opDiff",
                                                                    "opSmoothDiff", "opInter", "opSmoothInter", "opXor",
                                                                    "opSmoothXor"});

enum class SDFShaderPrim : uint8_t {
  Sphere = 0,
  Cube   = 1,
  _Count = 2,  // NOLINT
};
constexpr EnumMapping<SDFShaderPrim> kSDFShaderPrimFunctionNames({"sdSphere", "sdCube"});

enum class SDFShaderComponents : uint8_t {
  Transforms = 0,
  Materials  = 1,
  Spheres    = 2,
  Cubes      = 3,
  _Count     = 4,  // NOLINT
};
constexpr EnumMapping<SDFShaderComponents> kSDFShaderComponentArrayNames({
    "u_transforms",
    "u_materials",
    "u_spheres",
    "u_cubes",
});

enum class SDFShaderVariable : uint8_t {
  Position = 0,
  _Count   = 1,  // NOLINT
};
constexpr EnumMapping<SDFShaderVariable> kSDFShaderVariableNames({"pos"});

}  // namespace sdf_shader_consts

}  // namespace resin

#endif
