#ifndef RESIN_GROUP_NODE_HPP
#define RESIN_GROUP_NODE_HPP
#include <libresin/core/sdf/sdf_tree_node.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/transform.hpp>
#include <libresin/utils/logger.hpp>
#include <memory>
#include <string>
#include <vector>

namespace resin {

using SDFBinaryOperation = sdf_shader_consts::SDFShaderBinOp;

class GroupNode : public SDFTreeNode {
 public:
  GroupNode();
  virtual ~GroupNode();

  std::string gen_shader_code() const override;

  inline void accept_visitor(IMutableSDFTreeNodeVisitor& visitor) override { visitor.visit_group(*this); }
  inline void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) override { visitor.visit_group(*this); }

  template <SDFTreeNodeConcept Node, typename... Args>
    requires std::constructible_from<Node, Args...>
  inline void push_node(SDFBinaryOperation op, Args&&... args) {
    auto node_ptr = std::make_shared<Node>(std::forward<Args>(args)...);

    nodes_.emplace_back(op, std::move(node_ptr));
    nodes_.back().second->transform().set_parent(this->transform_);
  }

  inline void set_op(size_t node_id, SDFBinaryOperation op) { nodes_[node_id].first = op; }
  inline void set_node(size_t node_id, std::shared_ptr<SDFTreeNode> new_node) {
    nodes_[node_id].second = std::move(new_node);
  }

  auto begin() { return nodes_.begin(); }
  auto end() { return nodes_.end(); }
  auto begin() const { return nodes_.begin(); }
  auto end() const { return nodes_.end(); }

  inline std::string_view name() const override { return name_; }
  inline void rename(std::string&& name) override { name_ = std::move(name); }

 private:
  mutable std::vector<std::pair<SDFBinaryOperation, std::shared_ptr<SDFTreeNode>>> nodes_;
  std::string name_;
};

}  // namespace resin

#endif
