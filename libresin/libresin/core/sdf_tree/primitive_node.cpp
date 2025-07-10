#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>

namespace resin {

PrimitiveNode::PrimitiveNode(SDFTreeRegistry& tree, const SDFPrimitiveTypeDescription& desc)
    : SDFTreeNode(tree, desc.name),
      primitive_type_name_(desc.name),
      primitive_func_name_(desc.primitive_func_name),
      prim_id_(tree.primitives_registry) {
  mark_primitives_dirty();
  mark_dirty();
  for (const auto& param : desc.params) {
    params_.emplace(PrimitiveNodeParam{.name = param, .value = 1.F});
  }
}
PrimitiveNode::PrimitiveNode(SDFTreeRegistry& tree, std::string&& primitive_type_name,
                             std::string&& primitive_func_name, Params&& params)
    : SDFTreeNode(tree, primitive_type_name),
      primitive_type_name_(std::move(primitive_type_name)),
      primitive_func_name_(std::move(primitive_func_name)),
      params_(std::move(params)),
      prim_id_(tree.primitives_registry) {
  mark_primitives_dirty();
  mark_dirty();
}

std::unique_ptr<SDFTreeNode> PrimitiveNode::copy() {
  auto result = std::make_unique<PrimitiveNode>(tree_registry_, std::string(primitive_type_name_),
                                                std::string(primitive_func_name_), Params(params_));
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
      return std::format("{}({},{},{})", primitive_func_name_,
                         sdf_shader_consts::kSDFShaderVariableNames[sdf_shader_consts::SDFShaderVariable::Position],  //
                         node_id_.raw(),                                                                              //
                         prim_id_.raw()                                                                               //
      );
    case resin::GenShaderMode::ArrayPerPrimitiveType:
      break;
  }

  throw NonExhaustiveEnumException();
}

}  // namespace resin
