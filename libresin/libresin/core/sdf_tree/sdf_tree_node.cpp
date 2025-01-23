#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>

namespace resin {

SDFTreeNode::SDFTreeNode(SDFTreeRegistry& tree, std::string_view name)
    : node_id_(tree.nodes_registry),
      transform_id_(tree.transform_component_registry),
      bin_op_(SDFBinaryOperation::Union),
      tree_registry_(tree),
      name_(std::format("{} {}", name, tree.node_index++)) {
  tree_registry_.all_nodes[node_id_.raw()] = *this;
}

SDFTreeNode::~SDFTreeNode() {
  tree_registry_.all_nodes[node_id_.raw()] = std::nullopt;
  Logger::debug("Destructed node with id={}.", node_id_.raw());
}

void SDFTreeNode::set_bin_op(SDFBinaryOperation bin_op) {
  bin_op_                      = bin_op;
  tree_registry_.is_tree_dirty = true;
}

void SDFTreeNode::mark_dirty() { push_dirty_primitives(); }

}  // namespace resin
