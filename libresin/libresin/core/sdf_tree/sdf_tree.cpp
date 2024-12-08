#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/primitive_base_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/utils/exceptions.hpp>

namespace resin {
size_t SDFTree::curr_id_ = 0;

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

SDFTreeNode& SDFTree::node(IdView<SDFTreeNodeId> node_id) {
  if (!sdf_tree_registry_.all_nodes[node_id.raw()].has_value()) {
    log_throw(SDFTreeNodeDoesNotExist(node_id.raw()));
  }

  return sdf_tree_registry_.all_nodes[node_id.raw()].value();
}

GroupNode& SDFTree::group(IdView<SDFTreeNodeId> node_id) {
  if (!is_group(node_id)) {
    log_throw(SDFTreeNodeDoesNotExist(node_id.raw()));
  }

  return sdf_tree_registry_.all_group_nodes[node_id.raw()].value();
}

void SDFTree::visit_dirty_primitives(ISDFTreeNodeVisitor& visitor) {
  for (auto prim : sdf_tree_registry_.dirty_primitives) {
    sdf_tree_registry_.all_nodes[prim.raw()]->get().accept_visitor(visitor);
  }
  clear_dirty();
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

SDFTreeRegistry::SDFTreeRegistry()
    : sphere_components_registry(IdRegistry<PrimitiveNode<sdf_shader_consts::SDFShaderPrim::Sphere>>(1000)),
      cubes_components_registry(IdRegistry<PrimitiveNode<sdf_shader_consts::SDFShaderPrim::Cube>>(1000)),
      transform_component_registry(IdRegistry<Transform>(2000)),
      nodes_registry(IdRegistry<SDFTreeNode>(5000)) {
  all_nodes.resize(nodes_registry.get_max_objs());
  all_group_nodes.resize(nodes_registry.get_max_objs());
  dirty_primitives.reserve(nodes_registry.get_max_objs());
}

}  // namespace resin
