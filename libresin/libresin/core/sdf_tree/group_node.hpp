#ifndef RESIN_GROUP_NODE_HPP
#define RESIN_GROUP_NODE_HPP

#include <concepts>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/transform.hpp>
#include <libresin/utils/logger.hpp>
#include <memory>
#include <string>
#include <vector>

namespace resin {

class GroupNode : public SDFTreeNode {
 public:
  GroupNode() = delete;
  explicit GroupNode(SDFTreeRegistry& tree);
  virtual ~GroupNode() = default;

  std::string gen_shader_code() const override;
  std::string_view name() const override { return name_; }
  void rename(std::string&& name) override { name_ = std::move(name); }
  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override { visitor.visit_group(*this); }

  template <SDFTreeNodeConcept Node, typename... Args>
    requires std::constructible_from<Node, SDFTreeRegistry&, Args...>
  inline void push_node(SDFBinaryOperation op, Args&&... args) {
    auto node_ptr = std::make_unique<Node>(this->tree_registry_, std::forward<Args>(args)...);
    nodes_.emplace_back(std::move(node_ptr));
    nodes_.back()->set_bin_op(op);
    nodes_.back()->transform().set_parent(this->transform_);
  }

  auto begin() { return nodes_.begin(); }
  auto end() { return nodes_.end(); }
  auto begin() const { return nodes_.begin(); }
  auto end() const { return nodes_.end(); }

 private:
  std::vector<std::unique_ptr<SDFTreeNode>> nodes_;
  std::string name_;
};

}  // namespace resin

#endif
