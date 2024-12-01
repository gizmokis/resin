#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>

namespace resin {

GroupNode::GroupNode(SDFTreeRegistry& tree) : SDFTreeNode(tree), name_(std::format("Group {}", node_id_.raw())) {
  this->push_child<SphereNode>(SDFBinaryOperation::Union);
}

std::string GroupNode::gen_shader_code() const {
  if (this->nodes_.empty()) {
    // If group is empty then the operation is undefined.
    log_throw(SDFTreeEmptyGroupException());
  }

  if (this->nodes_.size() == 1) {
    // If there is one node only, the operation is ignored
    return this->get_child(*this->nodes_order_.begin()).gen_shader_code();
  }

  std::string sdf;
  for (auto it = this->nodes_order_.rbegin(); it != std::prev(this->nodes_order_.rend()); ++it) {
    sdf += sdf_shader_consts::kSDFShaderBinOpFunctionNames.get_name(this->get_child(*it).bin_op());
    sdf += "(";
  }
  sdf += this->get_child(*this->nodes_order_.begin()).gen_shader_code();
  sdf += ",";

  for (auto it = std::next(this->nodes_order_.begin()); it != this->nodes_order_.end(); ++it) {
    sdf += this->get_child(*it).gen_shader_code();
    sdf += "),";
  }
  sdf.pop_back();

  return sdf;
}

void GroupNode::delete_child(IdView<SDFTreeNodeId> node_id) {
  auto it = nodes_.find(node_id.raw());
  if (it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild());
  }

  nodes_order_.erase(it->second.first);
  nodes_.erase(it);
}

bool GroupNode::child_has_neighbor_up(IdView<SDFTreeNodeId> node_id) const {
  auto map_it = nodes_.find(node_id.raw());
  if (map_it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild());
  }

  return map_it->second.first != nodes_order_.begin();
}

bool GroupNode::child_has_neighbor_down(IdView<SDFTreeNodeId> node_id) const {
  auto map_it = nodes_.find(node_id.raw());
  if (map_it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild());
  }

  auto it = map_it->second.first;
  return ++it != nodes_order_.end();
}

SDFTreeNode& GroupNode::child_neighbor_up(IdView<SDFTreeNodeId> node_id) {
  auto map_it = nodes_.find(node_id.raw());
  if (map_it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild());
  }

  auto list_it = map_it->second.first;
  if (list_it == nodes_order_.begin()) {
    log_throw(SDFTreeNodeDoesNotExist());
  }

  return *nodes_.find((++list_it)->raw())->second.second;
}

SDFTreeNode& GroupNode::child_neighbor_down(IdView<SDFTreeNodeId> node_id) {
  auto map_it = nodes_.find(node_id.raw());
  if (map_it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild());
  }

  auto list_it = map_it->second.first;
  if (++list_it == nodes_order_.end()) {
    log_throw(SDFTreeNodeDoesNotExist());
  }

  return *nodes_.find(list_it->raw())->second.second;
}

}  // namespace resin
