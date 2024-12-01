#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>

#include "libresin/utils/exceptions.hpp"

namespace resin {

SDFTreeNode::SDFTreeNode(SDFTreeRegistry& tree)
    : node_id_(tree.nodes_registry),
      transform_id_(tree.transform_component_registry),
      bin_op_(SDFBinaryOperation::Union),
      tree_registry_(tree) {
  this->tree_registry_.get().all_nodes[node_id_.raw()] = *this;
}

SDFTreeNode::~SDFTreeNode() {
  this->tree_registry_.get().all_nodes[node_id_.raw()] = std::nullopt;
  Logger::debug("Destructed node with id={}.", this->node_id_.raw());
}

}  // namespace resin
