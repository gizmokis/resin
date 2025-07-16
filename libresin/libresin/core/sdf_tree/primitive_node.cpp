#include <libresin/core/resources/shader_type.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/utils/exceptions.hpp>

namespace resin {

PrimitiveNode::PrimitiveNode(SDFTreeRegistry& tree, const SDFPrimitiveTypeDescription& desc)
    : SDFTreeNode(tree, desc.name),
      type_id_(desc.id),
      type_name_(desc.name),
      glsl_sdf_name_(std::get<SDFShaderType>(desc.shader_res->type()).glsl_sdf_name),
      prim_id_(tree.primitives_registry) {
  mark_primitives_dirty();
  mark_dirty();
  for (const auto& param : desc.params) {
    params_.emplace(PrimitiveNodeParam{.name = param, .value = 1.F});
  }

  update_glsl_args(params_.size(), prim_id_.raw());
}

PrimitiveNode::PrimitiveNode(SDFTreeRegistry& tree, size_t primitive_type_id, std::string&& glsl_sdf_name,
                             std::string&& primitive_type_name, Params&& params)
    : SDFTreeNode(tree, primitive_type_name),
      type_id_(primitive_type_id),
      type_name_(std::move(primitive_type_name)),
      glsl_sdf_name_(std::move(glsl_sdf_name)),
      params_(std::move(params)),
      prim_id_(tree.primitives_registry) {
  mark_primitives_dirty();
  mark_dirty();
  update_glsl_args(params_.size(), prim_id_.raw());
}

void PrimitiveNode::update_glsl_args(size_t args_count, size_t prim_id) {
  if (args_count > 3) {
    throw TooManySDFPrimitiveParameters();
  }

  if (args_count == 1) {
    glsl_args_ = std::format(
        "({2}[{1}].transform*vec4({0}, 1)).xyz,{2}[{1}].size.x",
        sdf_shader_consts::kSDFShaderVariableNames[sdf_shader_consts::SDFShaderVariable::Position], prim_id,
        sdf_shader_consts::kSDFShaderCoreComponentArrayNames[sdf_shader_consts::SDFShaderCoreComponents::Primitives]);
  } else if (args_count == 2) {
    glsl_args_ = std::format(
        "({2}[{1}].transform*vec4({0}, 1)).xyz,{2}[{1}].size.x,{2}[{1}].size.y",
        sdf_shader_consts::kSDFShaderVariableNames[sdf_shader_consts::SDFShaderVariable::Position], prim_id,
        sdf_shader_consts::kSDFShaderCoreComponentArrayNames[sdf_shader_consts::SDFShaderCoreComponents::Primitives]);
  } else {
    glsl_args_ = std::format(
        "({2}[{1}].transform*vec4({0}, "
        "1)).xyz,{2}[{1}].size.x,{2}[{1}].size.y,{2}[{1}].size.z",
        sdf_shader_consts::kSDFShaderVariableNames[sdf_shader_consts::SDFShaderVariable::Position], prim_id,
        sdf_shader_consts::kSDFShaderCoreComponentArrayNames[sdf_shader_consts::SDFShaderCoreComponents::Primitives]);
  }
}

std::unique_ptr<SDFTreeNode> PrimitiveNode::copy() {
  auto result = std::make_unique<PrimitiveNode>(tree_registry_, type_id_, std::string(glsl_sdf_name_),
                                                std::string(type_name_), Params(params_));
  copy_common(*result, *this);
  return result;
}

void PrimitiveNode::fix_material_ancestors() {
  tree_registry_.is_tree_dirty = true;
  if (!parent_.has_value()) {
    ancestor_mat_id_ = std::nullopt;
  } else {
    ancestor_mat_id_ = parent_->get().active_material_id();
  }
}

std::string PrimitiveNode::gen_shader_code(GenShaderMode mode) const {
  switch (mode) {
    case resin::GenShaderMode::SinglePrimitiveArray:
      // example: createPrimitive(pos, Sphere_SDF(u_sdf_primitives[primitive_id].size.x), 1, 1)
      return std::format("{}({}({}),{},{})", sdf_shader_consts::kCreatePrimitiveFuncName, glsl_sdf_name_, glsl_args_,
                         node_id_.raw(),  //
                         prim_id_.raw()   //
      );
  }

  throw NonExhaustiveEnumException();
}

}  // namespace resin
