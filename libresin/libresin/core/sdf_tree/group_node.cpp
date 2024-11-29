#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>
#include <memory>
#include <ranges>

namespace resin {

GroupNode::GroupNode(SDFTreeRegistry& tree) : SDFTreeNode(tree), name_(std::format("Group {}", node_id_.raw())) {
  this->push_node<SphereNode>(SDFBinaryOperation::Union);
}

GroupNode::~GroupNode() {}

std::string GroupNode::gen_shader_code() const {
  if (this->nodes_.empty()) {
    // If group is empty then the operation is undefined.
    log_throw(SDFTreeEmptyGroupException());
  }

  if (this->nodes_.size() == 1) {
    // If there is one node only, the operation is ignored
    return this->nodes_[0].second->gen_shader_code();
  }

  std::string sdf;
  for (const auto& [fst, snd] : std::ranges::reverse_view(this->nodes_)) {
    sdf += sdf_shader_consts::kSDFShaderBinOpFunctionNames.get_name(fst);
    sdf += "(";
  }
  sdf += this->nodes_[0].second->gen_shader_code();

  for (const auto& val : this->nodes_ | std::views::values) {
    sdf += val->gen_shader_code();
    sdf += "),";
  }
  sdf.pop_back();

  return sdf;
}

}  // namespace resin
