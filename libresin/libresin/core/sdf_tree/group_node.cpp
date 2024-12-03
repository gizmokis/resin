#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>
#include <optional>

namespace resin {

GroupNode::GroupNode(SDFTreeRegistry& tree) : SDFTreeNode(tree), name_(std::format("Group {}", node_id_.raw())) {
  push_back_child<SphereNode>(SDFBinaryOperation::Union);
}

std::string GroupNode::gen_shader_code() const {
  if (nodes_.empty()) {
    // If group is empty then the operation is undefined.
    log_throw(SDFTreeEmptyGroupException());
  }

  if (nodes_.size() == 1) {
    // If there is one node only, the operation is ignored
    return get_child(*nodes_order_.begin()).gen_shader_code();
  }

  std::string sdf;
  for (auto it = nodes_order_.rbegin(); it != std::prev(nodes_order_.rend()); ++it) {
    sdf += sdf_shader_consts::kSDFShaderBinOpFunctionNames.get_name(get_child(*it).bin_op());
    sdf += "(";
  }
  sdf += get_child(*nodes_order_.begin()).gen_shader_code();
  sdf += ",";

  for (auto it = std::next(nodes_order_.begin()); it != nodes_order_.end(); ++it) {
    sdf += get_child(*it).gen_shader_code();
    sdf += "),";
  }
  sdf.pop_back();

  return sdf;
}

SDFTreeNode& GroupNode::get_child(IdView<SDFTreeNodeId> node_id) const {
  auto it = nodes_.find(node_id);
  if (it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild());
  }

  return *it->second.second;
}

void GroupNode::delete_child(IdView<SDFTreeNodeId> node_id) {
  auto it = nodes_.find(node_id);
  if (it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild());
  }

  nodes_order_.erase(it->second.first);
  nodes_.erase(it);
}

bool GroupNode::child_has_neighbor_prev(IdView<SDFTreeNodeId> node_id) const {
  auto map_it = nodes_.find(node_id);
  if (map_it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild());
  }

  return map_it->second.first != nodes_order_.begin();
}

bool GroupNode::child_has_neighbor_next(IdView<SDFTreeNodeId> node_id) const {
  auto map_it = nodes_.find(node_id);
  if (map_it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild());
  }

  auto it = map_it->second.first;
  return ++it != nodes_order_.end();
}

SDFTreeNode& GroupNode::child_neighbor_prev(IdView<SDFTreeNodeId> node_id) {
  auto map_it = nodes_.find(node_id);
  if (map_it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild());
  }

  auto list_it = map_it->second.first;
  if (list_it == nodes_order_.begin()) {
    log_throw(SDFTreeNodeDoesNotExist());
  }

  return *nodes_.find(*(++list_it))->second.second;
}

SDFTreeNode& GroupNode::child_neighbor_next(IdView<SDFTreeNodeId> node_id) {
  auto map_it = nodes_.find(node_id);
  if (map_it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild());
  }

  auto list_it = map_it->second.first;
  if (++list_it == nodes_order_.end()) {
    log_throw(SDFTreeNodeDoesNotExist());
  }

  return *nodes_.find(*list_it)->second.second;
}

void GroupNode::set_parent(std::unique_ptr<SDFTreeNode>& node_ptr) {
  node_ptr->set_parent(*this);
  node_ptr->transform().set_parent(transform_);
  insert_leaves_up(node_ptr);
}

void GroupNode::remove_from_parent(std::unique_ptr<SDFTreeNode>& node_ptr) {
  node_ptr->remove_from_parent();
  node_ptr->transform().remove_from_parent();
  remove_leaves_up(node_ptr);
}

std::unique_ptr<SDFTreeNode> GroupNode::detach_child(IdView<SDFTreeNodeId> node_id) {
  auto map_it = nodes_.find(node_id);
  if (map_it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild());
  }

  nodes_order_.erase(map_it->second.first);
  auto child_ptr = std::move(map_it->second.second);
  nodes_.erase(map_it);

  remove_from_parent(child_ptr);

  return child_ptr;
}

void GroupNode::push_back_child(std::unique_ptr<SDFTreeNode> node_ptr) {
  set_parent(node_ptr);
  auto node_id = node_ptr->node_id();

  nodes_order_.push_back(node_id);
  nodes_.emplace(node_id, std::make_pair(std::prev(nodes_order_.end()), std::move(node_ptr)));
}

void GroupNode::push_front_child(std::unique_ptr<SDFTreeNode> node_ptr) {
  set_parent(node_ptr);
  auto node_id = node_ptr->node_id();

  nodes_order_.push_front(node_id);
  nodes_.emplace(node_id, std::make_pair(std::prev(nodes_order_.end()), std::move(node_ptr)));
}

void GroupNode::insert_before_child(std::optional<IdView<SDFTreeNodeId>> before_child_id,
                                    std::unique_ptr<SDFTreeNode> node_ptr) {
  if (!before_child_id.has_value()) {
    push_back_child(std::move(node_ptr));
    return;
  }
  set_parent(node_ptr);

  auto map_it = nodes_.find(*before_child_id);
  if (map_it != nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild());
  }

  auto list_it = nodes_order_.emplace(map_it->second.first, node_ptr->node_id());
  nodes_.emplace(node_ptr->node_id(), std::make_pair(list_it, std::move(node_ptr)));
}

void GroupNode::insert_after_child(std::optional<IdView<SDFTreeNodeId>> after_child_id,
                                   std::unique_ptr<SDFTreeNode> node_ptr) {
  if (!after_child_id.has_value()) {
    push_front_child(std::move(node_ptr));
    return;
  }
  set_parent(node_ptr);

  auto map_it = nodes_.find(*after_child_id);
  if (map_it != nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild());
  }

  auto list_it = nodes_order_.emplace(map_it->second.first, node_ptr->node_id());
  nodes_.emplace(node_ptr->node_id(), std::make_pair(++list_it, std::move(node_ptr)));
}

void GroupNode::mark_dirty() {
  if (tree_registry_.get().nodes_registry.get_max_objs() < tree_registry_.get().dirty_primitives.size()) {
    log_throw(SDFTreeReachedDirtyPrimitivesLimit());
  }

  for (auto prim : leaves_) {
    tree_registry_.get().dirty_primitives.push_back(std::move(prim));
  }
}

std::unique_ptr<SDFTreeNode> GroupNode::copy() {
  auto result = make_unique<GroupNode>(tree_registry_);
  for (auto& list_it : nodes_order_) {
    result->push_back_child(nodes_.find(list_it)->second.second->copy());
  }

  return result;
}

void GroupNode::insert_leaves_up(const std::unique_ptr<SDFTreeNode>& source) {
  source->insert_leaves_to(leaves_);
  if (parent_.has_value()) {
    parent_->get().insert_leaves_up(source);
  }
}

void GroupNode::remove_leaves_up(const std::unique_ptr<SDFTreeNode>& source) {
  source->remove_leaves_from(leaves_);
  if (parent_.has_value()) {
    parent_->get().remove_leaves_up(source);
  }
}

}  // namespace resin
