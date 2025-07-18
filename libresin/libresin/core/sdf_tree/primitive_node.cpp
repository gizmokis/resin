#include <libresin/core/resources/shader_type.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_primitive_type_manager.hpp>
#include <libresin/utils/exceptions.hpp>

namespace resin {

PrimitiveNode::PrimitiveNode(SDFTreeRegistry& tree, std::optional<uint32_t> primitive_type_id)
    : SDFTreeNode(tree, "Primitive"), prim_id_(tree.primitives_registry), type_id_(primitive_type_id) {
  update_id();

  if (type_id_) {
    for (const auto& param : tree_registry_.primitive_type_manager_.type_by_id(*type_id_).params) {
      params_.emplace(PrimitiveNodeParam{.name = param, .value = 1.F});
    }
  }
  mark_primitives_dirty();
  mark_dirty();
  update_glsl_args(params_.size(), prim_id_.raw());
  tree_registry_.all_primitive_nodes[node_id_.raw()] = *this;
}

PrimitiveNode::~PrimitiveNode() { tree_registry_.all_primitive_nodes[node_id_.raw()] = std::nullopt; }

void PrimitiveNode::update_id() const {
  if (type_id_ && !tree_registry_.primitive_type_manager_.is_id_valid(*type_id_)) {
    type_id_ = std::nullopt;
  }
}

void PrimitiveNode::set_type_id(std::optional<uint32_t> new_type_id) {
  type_id_ = new_type_id;
  update_id();
}

optional_ref<const SDFPrimitiveTypeDescription> PrimitiveNode::type() const {
  update_id();
  if (!type_id_) {
    return std::nullopt;
  }

  return tree_registry_.primitive_type_manager_.type_by_id(*type_id_);
}

std::optional<uint32_t> PrimitiveNode::type_id() const {
  update_id();

  return type_id_;
}

void PrimitiveNode::update_glsl_args(size_t args_count, size_t prim_id) {
  if (args_count > sdf_shader_consts::kSDFMaxParamCount) {
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
  update_id();

  auto result     = std::make_unique<PrimitiveNode>(tree_registry_, type_id_);
  result->params_ = params_;
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
  update_id();

  switch (mode) {
    case resin::GenShaderMode::SinglePrimitiveArray:
      // This unsafe call is okay since SDFPrimitiveTypeDescription always contain the SDF shader
      if (type_id_) {
        std::string_view glsl_sdf_name =
            std::get<SDFShaderType>(tree_registry_.primitive_type_manager_.type_by_id(*type_id_).shader_res->type())
                .glsl_sdf_name;

        // example: createPrimitive(pos, Sphere_SDF(u_sdf_primitives[primitive_id].size.x), 1, 1)
        return std::format("{}({}({}),{},{})", sdf_shader_consts::kCreatePrimitiveFuncName, glsl_sdf_name, glsl_args_,
                           node_id_.raw(),  //
                           prim_id_.raw()   //
        );
      }

      return std::string(sdf_shader_consts::kCreateEmptyPrimitiveFuncCall);
  }

  throw NonExhaustiveEnumException();
}

}  // namespace resin
