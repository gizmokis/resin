#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <memory>

#include "libresin/utils/exceptions.hpp"

namespace resin {

void SDFTree::visit_node(IdView<SDFTreeNodeId> node_id, ISDFTreeNodeVisitor& visitor) {
  if (!sdf_tree_registry_.all_nodes[node_id.raw()].has_value()) {
    log_throw(SDFTreeNodeDoesNotExist());
  }
  sdf_tree_registry_.all_nodes[node_id.raw()]->get().accept_visitor(visitor);
}

void SDFTree::delete_node(IdView<SDFTreeNodeId> node_id) {
  if (!sdf_tree_registry_.all_nodes[node_id.raw()].has_value()) {
    log_throw(SDFTreeNodeDoesNotExist());
  }

  auto parent = sdf_tree_registry_.all_nodes[node_id.raw()]->get().get_parent();
  if (!parent.has_value()) {
    log_throw(SDFTreeRootDeletionError());
  }

  parent.value().get().delete_child(node_id);
}

std::unique_ptr<SDFTreeNode>& SDFTree::get_node(IdView<SDFTreeNodeId> node_id) {
  if (!sdf_tree_registry_.all_nodes[node_id.raw()].has_value()) {
    log_throw(SDFTreeNodeDoesNotExist());
  }

  auto parent = sdf_tree_registry_.all_nodes[node_id.raw()]->get().get_parent();
  if (!parent.has_value()) {
    return reinterpret_cast<std::unique_ptr<SDFTreeNode>&>(this->root);  // GroupNode derives from SDFTreeNode
  }

  return parent->get().get_child(node_id);
}

}  // namespace resin
