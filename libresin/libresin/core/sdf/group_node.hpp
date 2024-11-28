#ifndef RESIN_GROUP_NODE_HPP
#define RESIN_GROUP_NODE_HPP
#include <libresin/core/sdf/sdf_tree_node.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/transform.hpp>
#include <memory>
#include <vector>

namespace resin {

using SDFBinaryOperation = sdf_shader_consts::SDFShaderBinOp;

class GroupNode : public SDFTreeNode {
 public:
  std::string gen_shader_code() const override;

  GroupNode();
  ~GroupNode() override = default;

  inline void push_node(SDFBinaryOperation op, std::unique_ptr<SDFTreeNode> node) {
    nodes_.emplace_back(op, std::move(node));
  }
  template <SDFTreeNodeConcept Node>
  inline void push_node(SDFBinaryOperation op) {
    nodes_.emplace_back(op, std::make_unique<Node>());
  }

  inline void set_op(size_t node_id, SDFBinaryOperation op) { nodes_[node_id].first = op; }
  inline void set_node(size_t node_id, std::unique_ptr<SDFTreeNode> new_node) {
    nodes_[node_id].second = std::move(new_node);
  }

  inline void accept_visitor(IMutableSDFTreeNodeVisitor& visitor) override { visitor.visit_group(*this); }
  inline void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) override { visitor.visit_group(*this); }

  auto begin() { return nodes_.begin(); }
  auto end() { return nodes_.end(); }
  auto begin() const { return nodes_.begin(); }
  auto end() const { return nodes_.end(); }

  inline std::string_view get_name() const { return name_; }

 private:
  mutable std::vector<std::pair<SDFBinaryOperation, std::unique_ptr<SDFTreeNode>>> nodes_;
  std::string name_;
};

}  // namespace resin

#endif
