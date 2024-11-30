#ifndef RESIN_GROUP_NODE_HPP
#define RESIN_GROUP_NODE_HPP

#include <concepts>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/transform.hpp>
#include <libresin/utils/logger.hpp>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "libresin/core/id_registry.hpp"

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
  inline IdView<SDFTreeNodeId> push_child(SDFBinaryOperation op, Args&&... args) {
    auto node_ptr = std::make_unique<Node>(this->tree_registry_, std::forward<Args>(args)...);
    node_ptr->set_bin_op(op);
    node_ptr->transform().set_parent(this->transform_);
    node_ptr->set_parent(*this);

    auto node_id = node_ptr->node_id();

    nodes_order_.push_back(node_id);
    nodes_.emplace(node_id.raw(), std::make_pair(std::prev(nodes_order_.end()), std::move(node_ptr)));

    return node_id;
  }

  template <SDFTreeNodeConcept Node, typename... Args>
    requires std::constructible_from<Node, SDFTreeRegistry&, Args...>
  void insert_before(IdView<SDFTreeNodeId> node_id, SDFBinaryOperation op) {}

  template <SDFTreeNodeConcept Node, typename... Args>
    requires std::constructible_from<Node, SDFTreeRegistry&, Args...>
  void insert_after(IdView<SDFTreeNodeId> node_id, SDFBinaryOperation op) {}

  void move_child_after(IdView<SDFTreeNodeId> after_child_id, IdView<SDFTreeNodeId> child_id);
  void move_child_before(IdView<SDFTreeNodeId> before_child_id, IdView<SDFTreeNodeId> child_id);
  void move_child_up(IdView<SDFTreeNodeId> node_id);
  void move_child_down(IdView<SDFTreeNodeId> node_id);
  void reorder_children(std::vector<IdView<SDFTreeNodeId>>&& new_order);

  // Cost: Amortized O(1)
  // WARNING: This function must not be called while the children are iterated. Use GroupNode::erase_child in this
  // scenario.
  void delete_child(IdView<SDFTreeNodeId> node_id);

  // Cost: Amortized O(1)
  auto erase_child(std::list<IdView<SDFTreeNodeId>>::iterator it) {
    auto map_it = nodes_.find(it->raw());
    if (map_it == nodes_.end()) {
      log_throw(SDFTreeNodeIsNotAChild());
    }
    auto next = nodes_order_.erase(it);
    nodes_.erase(map_it);

    return next;
  }

  // TODO(migoox): write safe children iterator
  auto begin() { return nodes_order_.begin(); }
  auto end() { return nodes_order_.end(); }
  auto begin() const { return nodes_order_.begin(); }
  auto end() const { return nodes_order_.end(); }

  std::unique_ptr<SDFTreeNode>& get_child(IdView<SDFTreeNodeId> node_id) {
    auto it = nodes_.find(node_id.raw());
    if (it == nodes_.end()) {
      log_throw(SDFTreeNodeIsNotAChild());
    }

    return it->second.second;
  }

  const std::unique_ptr<SDFTreeNode>& get_child(IdView<SDFTreeNodeId> node_id) const {
    auto it = nodes_.find(node_id.raw());
    if (it == nodes_.end()) {
      log_throw(SDFTreeNodeIsNotAChild());
    }

    return it->second.second;
  }

 private:
  std::list<IdView<SDFTreeNodeId>> nodes_order_;
  std::unordered_map<size_t, std::pair<std::list<IdView<SDFTreeNodeId>>::iterator, std::unique_ptr<SDFTreeNode>>>
      nodes_;

  std::string name_;
};

}  // namespace resin

#endif
