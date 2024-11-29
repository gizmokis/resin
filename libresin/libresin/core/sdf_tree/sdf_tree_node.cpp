#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>

namespace resin {

SDFTreeNode::SDFTreeNode(SDFTreeRegistry& tree)
    : node_id_(tree.nodes_registry_), transform_id_(tree.transform_component_registry_), tree_registry_(tree) {
  this->tree_registry_.get().all_nodes_[node_id_.raw()] = *this;
}

SDFTreeNode::~SDFTreeNode() {
  this->tree_registry_.get().all_nodes_[node_id_.raw()] = std::nullopt;
  Logger::debug("Destructed node with id={}.", this->node_id_.raw());
}

}  // namespace resin
