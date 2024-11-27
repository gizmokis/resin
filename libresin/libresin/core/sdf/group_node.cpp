#include <libresin/core/sdf/group_node.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/utils/exceptions.hpp>
#include <ranges>

namespace resin {
std::string GroupNode::gen_shader_code() const {
  this->remove_expired();

  if (this->nodes_.empty()) {
    // If group is empty then the operation is undefined.
    log_throw(SDFTreeEmptyGroupException());
  }

  if (this->nodes_.size() == 1) {
    // If there is one node only, the operation is ignored
    return this->nodes_[0].second.lock()->gen_shader_code();
  }

  std::string sdf;
  for (const auto& node : std::ranges::reverse_view(this->nodes_)) {
    sdf += sdf_shader_consts::kSDFShaderBinOpFunctionNames.get_name(node.first);
    sdf += "(";
  }
  sdf += this->nodes_[0].second.lock()->gen_shader_code();

  for (const auto& node : this->nodes_) {
    sdf += node.second.lock()->gen_shader_code();
    sdf += "),";
  }
  sdf.pop_back();

  return sdf;
}

void GroupNode::set_op(int node_id, SDFBinaryOperation op) { log_throw(NotImplementedException()); }
void GroupNode::set_obj(int node_id, std::weak_ptr<const SDFTreeNode>) { log_throw(NotImplementedException()); }
void GroupNode::remove_expired() const {}

}  // namespace resin
