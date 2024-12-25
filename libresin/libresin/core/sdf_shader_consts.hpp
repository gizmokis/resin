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
constexpr StringEnumMapping<SDFShaderBinOp> kSDFShaderBinOpFunctionNames({
    {SDFShaderBinOp::Union, "opUnion"},              //
    {SDFShaderBinOp::SmoothUnion, "opSmoothUnion"},  //
    {SDFShaderBinOp::Diff, "opDiff"},                //
    {SDFShaderBinOp::SmoothDiff, "opSmoothDiff"},    //
    {SDFShaderBinOp::Inter, "opInter"},              //
    {SDFShaderBinOp::SmoothInter, "opSmoothInter"},  //
    {SDFShaderBinOp::Xor, "opXor"},                  //
    {SDFShaderBinOp::SmoothXor, "opSmoothXor"},      //
});

enum class SDFShaderPrim : uint8_t {
  Sphere = 0,
  Cube   = 1,
  _Count = 2,  // NOLINT
};
constexpr StringEnumMapping<SDFShaderPrim> kSDFShaderPrimFunctionNames({
    {SDFShaderPrim::Sphere, "sdSphere"},  //
    {SDFShaderPrim::Cube, "sdCube"}       //
});

constexpr StringEnumMapping<SDFShaderPrim> kSDFShaderPrimComponentArrayNames({
    {SDFShaderPrim::Sphere, "u_spheres"},  //
    {SDFShaderPrim::Cube, "u_cubes"}       //
});

enum class SDFShaderCoreComponents : uint8_t {
  Transforms = 0,
  Materials  = 1,
  _Count     = 2,  // NOLINT
};
constexpr StringEnumMapping<SDFShaderCoreComponents> kSDFShaderCoreComponentArrayNames({
    {SDFShaderCoreComponents::Transforms, "u_transforms"},  //
    {SDFShaderCoreComponents::Materials, "u_materials"}     //
});

enum class SDFShaderVariable : uint8_t {
  Position = 0,
  _Count   = 1,  // NOLINT
};
constexpr StringEnumMapping<SDFShaderVariable> kSDFShaderVariableNames({
    {SDFShaderVariable::Position, "pos"},  //
});

}  // namespace sdf_shader_consts

}  // namespace resin

#endif
