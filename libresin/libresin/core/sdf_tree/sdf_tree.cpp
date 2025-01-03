#include <libresin/core/id_registry.hpp>
#include <libresin/core/material.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_base_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/utils/exceptions.hpp>
#include <optional>
#include <utility>

namespace resin {
size_t SDFTree::curr_id_ = 0;

SDFTree::SDFTree() : root_(std::make_unique<GroupNode>(sdf_tree_registry_)), tree_id_((curr_id_++)) {
  materials_.resize(sdf_tree_registry_.materials_registry.get_max_objs());
}

std::optional<IdView<SDFTreeNodeId>> SDFTree::get_view_from_raw_id(size_t raw_id) {
  if (!sdf_tree_registry_.all_nodes[raw_id].has_value()) {
    return sdf_tree_registry_.all_nodes[raw_id].value().get().node_id();
  }
  return std::nullopt;
}

void SDFTree::visit_node(IdView<SDFTreeNodeId> node_id, ISDFTreeNodeVisitor& visitor) {
  if (!sdf_tree_registry_.all_nodes[node_id.raw()].has_value()) {
    log_throw(SDFTreeNodeDoesNotExist(node_id.raw()));
  }
  sdf_tree_registry_.all_nodes[node_id.raw()]->get().accept_visitor(visitor);
}

const SDFTreeNode& SDFTree::node(IdView<SDFTreeNodeId> node_id) const {
  if (!sdf_tree_registry_.all_nodes[node_id.raw()].has_value()) {
    log_throw(SDFTreeNodeDoesNotExist(node_id.raw()));
  }

  return sdf_tree_registry_.all_nodes[node_id.raw()].value();
}

SDFTreeNode& SDFTree::node(IdView<SDFTreeNodeId> node_id) {
  return const_cast<SDFTreeNode&>(std::as_const(*this).node(node_id));  // NOLINT
}

const GroupNode& SDFTree::group(IdView<SDFTreeNodeId> node_id) const {
  if (!is_group(node_id)) {
    log_throw(SDFTreeNodeDoesNotExist(node_id.raw()));
  }

  return sdf_tree_registry_.all_group_nodes[node_id.raw()].value();
}

GroupNode& SDFTree::group(IdView<SDFTreeNodeId> node_id) {
  return const_cast<GroupNode&>(std::as_const(*this).group(node_id));  // NOLINT
}

void SDFTree::visit_dirty_primitives(ISDFTreeNodeVisitor& visitor) {
  for (auto prim : sdf_tree_registry_.dirty_primitives) {
    sdf_tree_registry_.all_nodes[prim.raw()]->get().accept_visitor(visitor);
  }
  mark_primitives_clean();
}

void SDFTree::visit_all_primitives(ISDFTreeNodeVisitor& visitor) {
  for (auto prim : root_->primitives()) {
    sdf_tree_registry_.all_nodes[prim.raw()]->get().accept_visitor(visitor);
  }
}

void SDFTree::delete_node(IdView<SDFTreeNodeId> node_id) {
  if (!sdf_tree_registry_.all_nodes[node_id.raw()].has_value()) {
    log_throw(SDFTreeNodeDoesNotExist(node_id.raw()));
  }

  if (!sdf_tree_registry_.all_nodes[node_id.raw()]->get().has_parent()) {
    log_throw(SDFTreeRootDeletionError());
  }

  sdf_tree_registry_.all_nodes[node_id.raw()]->get().parent().delete_child(node_id);
}

std::string SDFTree::gen_shader_code(GenShaderMode mode) const {
  std::string root_code = root_->gen_shader_code(mode);
  return root_code.empty() ? "vec2(u_farPlane,0)" : root_code;
}

const MaterialSDFTreeComponent& SDFTree::material(IdView<MaterialId> mat_id) const {
  if (mat_id == sdf_tree_registry_.default_material.material_id()) {
    return sdf_tree_registry_.default_material;
  }
  if (!materials_[mat_id.raw()].has_value()) {
    log_throw(MaterialSDFTreeComponentDoesNotExist(mat_id.raw()));
  }
  return *materials_[mat_id.raw()];
}

MaterialSDFTreeComponent& SDFTree::material(IdView<MaterialId> mat_id) {
  return const_cast<MaterialSDFTreeComponent&>(std::as_const(*this).material(mat_id));  // NOLINT
}

MaterialSDFTreeComponent& SDFTree::add_material(Material mat) {
  auto new_mat = MaterialSDFTreeComponent(sdf_tree_registry_, std::move(mat));
  auto id      = new_mat.material_id();
  material_active_ids_.push_back(id);
  materials_[id.raw()] = std::move(new_mat);
  return *materials_[id.raw()];
}

void SDFTree::delete_material(IdView<MaterialId> mat_id) {
  if (mat_id == sdf_tree_registry_.default_material.material_id()) {
    log_throw(DefaultMaterialDeletionAttempted());
  }

  root_->remove_material_from_subtree(mat_id);

  for (auto it = material_active_ids_.begin(); it != material_active_ids_.end(); ++it) {  // 😢
    if (*it != mat_id) {
      continue;
    }

    auto it2 = material_active_ids_.erase(it);  // 😢
    if (it2 != it) {
      break;
    }
  }

  materials_[mat_id.raw()] = std::nullopt;
}

void SDFTree::visit_dirty_materials(const std::function<void(MaterialSDFTreeComponent&)>& mat_visitor) {
  for (const auto& mat_id : sdf_tree_registry_.dirty_materials) {
    if (materials_[mat_id.raw()].has_value()) {
      mat_visitor(*materials_[mat_id.raw()]);
    }
  }
}

}  // namespace resin
