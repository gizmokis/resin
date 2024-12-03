#ifndef RESIN_GROUP_NODE_HPP
#define RESIN_GROUP_NODE_HPP

#include <algorithm>
#include <concepts>
#include <functional>
#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/transform.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace resin {

class GroupNode final : public SDFTreeNode {
 public:
  GroupNode() = delete;
  explicit GroupNode(SDFTreeRegistry& tree);
  virtual ~GroupNode() = default;

  std::string gen_shader_code() const override;
  std::string_view name() const override { return name_; }
  void rename(std::string&& name) override { name_ = std::move(name); }
  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override { visitor.visit_group(*this); }
  void mark_dirty() override;
  std::unique_ptr<SDFTreeNode> copy() override;

  inline size_t get_children_count() const { return this->nodes_.size(); }

  // Cost: O(h)
  template <SDFTreeNodeConcept Node, typename... Args>
    requires std::constructible_from<Node, SDFTreeRegistry&, Args...>
  inline void push_back_child(SDFBinaryOperation op, Args&&... args) {
    auto node_ptr = std::make_unique<Node>(this->tree_registry_, std::forward<Args>(args)...);
    node_ptr->set_bin_op(op);
    this->push_back_child(std::move(node_ptr));
  }

  // Cost: O(h)
  template <SDFTreeNodeConcept Node, typename... Args>
    requires std::constructible_from<Node, SDFTreeRegistry&, Args...>
  inline void push_front_child(SDFBinaryOperation op, Args&&... args) {
    auto node_ptr = std::make_unique<Node>(this->tree_registry_, std::forward<Args>(args)...);
    node_ptr->set_bin_op(op);
    this->push_front_child(std::move(node_ptr));
  }

  // Cost: O(h)
  void push_back_child(std::unique_ptr<SDFTreeNode> node_ptr);

  // Cost: O(h)
  void push_front_child(std::unique_ptr<SDFTreeNode> node_ptr);

  // If after_node_id is nullopt, the node_ptr is pushed back.
  // Cost: O(h)
  void insert_before_child(std::optional<IdView<SDFTreeNodeId>> before_child_id, std::unique_ptr<SDFTreeNode> node_ptr);

  // If before_node_id is nullopt, the node_ptr is pushed front.
  // Cost: O(h)
  void insert_after_child(std::optional<IdView<SDFTreeNodeId>> after_child_id, std::unique_ptr<SDFTreeNode> node_ptr);

  // Cost: Amortized O(1)
  bool child_has_neighbor_prev(IdView<SDFTreeNodeId> node_id) const;

  // Cost: Amortized O(1)
  SDFTreeNode& child_neighbor_prev(IdView<SDFTreeNodeId> node_id);

  // Cost: Amortized O(1)
  bool child_has_neighbor_next(IdView<SDFTreeNodeId> node_id) const;

  // Cost: Amortized O(1)
  SDFTreeNode& child_neighbor_next(IdView<SDFTreeNodeId> node_id);

  // Cost: Amortized O(1)
  // WARNING: This function must not be called while the children are iterated.
  std::unique_ptr<SDFTreeNode> detach_child(IdView<SDFTreeNodeId> node_id);

  // Cost: O(h)
  // WARNING: This function must not be called while the children are iterated.
  void delete_child(IdView<SDFTreeNodeId> node_id);

  // Cost: Amortized O(1)
  SDFTreeNode& get_child(IdView<SDFTreeNodeId> node_id) const;

  inline const std::unordered_set<IdView<SDFTreeNodeId>, IdViewHash<SDFTreeNodeId>, std::equal_to<>>& primitives() {
    return leaves_;
  }

  auto begin() { return nodes_order_.begin(); }
  auto end() { return nodes_order_.end(); }
  auto begin() const { return nodes_order_.begin(); }
  auto end() const { return nodes_order_.end(); }

 protected:
  inline void insert_leaves_to(
      std::unordered_set<IdView<SDFTreeNodeId>, IdViewHash<SDFTreeNodeId>, std::equal_to<>>& leaves) override {
    leaves.insert(this->leaves_.begin(), this->leaves_.end());
  }

  inline void remove_leaves_from(
      std::unordered_set<IdView<SDFTreeNodeId>, IdViewHash<SDFTreeNodeId>, std::equal_to<>>& leaves) override {
    for (const auto& leaf : this->leaves_) {
      leaves.erase(leaf);
    }
  }

 private:
  void insert_leaves_up(const std::unique_ptr<SDFTreeNode>& source);
  void remove_leaves_up(const std::unique_ptr<SDFTreeNode>& source);

  void set_parent(std::unique_ptr<SDFTreeNode>& node_ptr);
  void remove_from_parent(std::unique_ptr<SDFTreeNode>& node_ptr);

 private:
  std::list<IdView<SDFTreeNodeId>> nodes_order_;
  std::unordered_map<IdView<SDFTreeNodeId>,
                     std::pair<std::list<IdView<SDFTreeNodeId>>::iterator, std::unique_ptr<SDFTreeNode>>,
                     IdViewHash<SDFTreeNodeId>, std::equal_to<>>
      nodes_;

  std::unordered_set<IdView<SDFTreeNodeId>, IdViewHash<SDFTreeNodeId>, std::equal_to<>> leaves_;  // leaves

  std::string name_;
};

}  // namespace resin

#endif
