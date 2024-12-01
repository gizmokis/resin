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

bool SDFTreeNode::has_neighbor_up() const {
  if (!this->parent().has_parent()) {
    return false;
  }

  return this->parent().child_has_neighbor_up(this->node_id_);
}

bool SDFTreeNode::has_neighbor_down() const {
  if (!this->parent_.has_value()) {
    return false;
  }

  return this->parent_->get().child_has_neighbor_down(this->node_id_);
}

SDFTreeNode& SDFTreeNode::neighbor_down() {
  if (!this->parent_.has_value()) {
    log_throw(SDFTreeNodeDoesNotExist());
  }

  return this->parent_->get().child_neighbor_down(this->node_id());
}

SDFTreeNode& SDFTreeNode::neighbor_down() const {
  if (!this->parent_.has_value()) {
    log_throw(SDFTreeNodeDoesNotExist());
  }

  return this->parent_->get().child_neighbor_down(this->node_id());
}

SDFTreeNode& SDFTreeNode::neighbor_up() {
  if (!this->parent_.has_value()) {
    log_throw(SDFTreeNodeDoesNotExist());
  }

  return this->parent_->get().child_neighbor_up(this->node_id());
}

SDFTreeNode& SDFTreeNode::neighbor_up() const {
  if (!this->parent_.has_value()) {
    log_throw(SDFTreeNodeDoesNotExist());
  }

  return this->parent_->get().child_neighbor_up(this->node_id());
}

}  // namespace resin
