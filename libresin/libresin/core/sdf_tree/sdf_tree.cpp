#include <libresin/core/sdf_tree/sdf_tree.hpp>

#include "libresin/utils/exceptions.hpp"

namespace resin {

void SDFTree::visit_node(IdView<SDFTreeNodeId> node_id, ISDFTreeNodeVisitor& visitor) {
  if (!sdf_tree_registry_.all_nodes[node_id.raw()].has_value()) {
    log_throw(SDFTreeNodeDoesNotExist());
  }
  sdf_tree_registry_.all_nodes[node_id.raw()]->get().accept_visitor(visitor);
}

void visit_dirty_primitives(ISDFTreeNodeVisitor& visitor) { throw NotImplementedException(); }

void SDFTree::delete_node(IdView<SDFTreeNodeId> node_id) {
  if (!sdf_tree_registry_.all_nodes[node_id.raw()].has_value()) {
    log_throw(SDFTreeNodeDoesNotExist());
  }

  if (!sdf_tree_registry_.all_nodes[node_id.raw()]->get().has_parent()) {
    log_throw(SDFTreeRootDeletionError());
  }

  sdf_tree_registry_.all_nodes[node_id.raw()]->get().parent().delete_child(node_id);
}

void SDFTree::move_node_before(IdView<SDFTreeNodeId> before_node_id, IdView<SDFTreeNodeId> node_id) {
  throw NotImplementedException();
}

void SDFTree::move_node_after(IdView<SDFTreeNodeId> after_node_id, IdView<SDFTreeNodeId> node_id) {
  throw NotImplementedException();
}

void SDFTree::copy_node_before(IdView<SDFTreeNodeId> before_node_id, IdView<SDFTreeNodeId> node_id) {
  throw NotImplementedException();
}

void SDFTree::copy_node_after(IdView<SDFTreeNodeId> after_node_id, IdView<SDFTreeNodeId> node_id) {
  throw NotImplementedException();
}

std::string SDFTree::gen_shader_code() const { return this->root->gen_shader_code(); }

}  // namespace resin
