#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>
#include <ranges>

namespace resin {

GroupNode::GroupNode(SDFTreeRegistry& tree) : SDFTreeNode(tree), name_(std::format("Group {}", node_id_.raw())) {
  this->push_node<SphereNode>(SDFBinaryOperation::Union);
}

std::string GroupNode::gen_shader_code() const {
  if (this->nodes_.empty()) {
    // If group is empty then the operation is undefined.
    log_throw(SDFTreeEmptyGroupException());
  }

  if (this->nodes_.size() == 1) {
    // If there is one node only, the operation is ignored
    return this->nodes_[0]->gen_shader_code();
  }

  std::string sdf;
  for (auto it = this->nodes_.rbegin(); it != std::prev(this->nodes_.rend()); ++it) {
    sdf += sdf_shader_consts::kSDFShaderBinOpFunctionNames.get_name((*it)->bin_op());
    sdf += "(";
  }

  sdf += this->nodes_[0]->gen_shader_code();
  sdf += ",";

  for (auto it = std::next(this->nodes_.begin()); it != this->nodes_.end(); ++it) {
    sdf += (*it)->gen_shader_code();
    sdf += "),";
  }
  sdf.pop_back();

  return sdf;
}

}  // namespace resin
