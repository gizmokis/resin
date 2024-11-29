#include <libresin/core/sdf_tree/sdf_tree.hpp>

namespace resin {

bool SDFTree::visit_node(IdView<SDFTreeNodeId> node_id, ISDFTreeNodeVisitor& visitor) {
  if (!sdf_tree_registry_.all_nodes[node_id.raw()].has_value()) {
    return false;
  }
  sdf_tree_registry_.all_nodes[node_id.raw()]->get().accept_visitor(visitor);
  return true;
}

}  // namespace resin
