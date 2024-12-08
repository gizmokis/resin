#ifndef RESIN_SDF_TREE_NODE_HPP
#define RESIN_SDF_TREE_NODE_HPP

#include <functional>
#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <libresin/core/transform.hpp>
#include <string>
#include <unordered_set>

namespace resin {

using SDFBinaryOperation = sdf_shader_consts::SDFShaderBinOp;

struct SDFTreeRegistry;
class SDFTree;

class SDFTreeNode;
using SDFTreeNodeId = Id<SDFTreeNode>;

class SDFTreeNode {
 public:
  SDFTreeNode() = delete;

  // It is the programmer's responsibility to assert that SDFTreeNode class will not outlive the provided registry!
  explicit SDFTreeNode(SDFTreeRegistry& tree, std::string_view name);

  SDFTreeNode(const SDFTreeNode&)            = delete;
  SDFTreeNode& operator=(const SDFTreeNode&) = delete;

  virtual ~SDFTreeNode();

  virtual std::string gen_shader_code() const               = 0;
  virtual void accept_visitor(ISDFTreeNodeVisitor& visitor) = 0;
  [[nodiscard]] virtual std::unique_ptr<SDFTreeNode> copy() = 0;
  virtual bool is_leaf()                                    = 0;

  inline IdView<SDFTreeNodeId> node_id() const { return node_id_; }
  inline IdView<TransformId> transform_component_id() const { return transform_id_; }
  inline Transform& transform() { return transform_; }

  // TODO(SDF-96): add material component

  inline SDFBinaryOperation bin_op() const { return bin_op_; }
  inline void set_bin_op(SDFBinaryOperation bin_op) { bin_op_ = bin_op; }

  inline bool has_parent() const { return parent_.has_value(); }
  inline GroupNode& parent() { return parent_.value(); }
  inline const GroupNode& parent() const { return parent_.value(); }

  std::string_view name() const { return name_; }
  void rename(std::string&& name) { name_ = std::move(name); }

  void mark_dirty();

 protected:
  friend SDFTree;
  friend GroupNode;

  inline void set_parent(GroupNode& parent) { parent_ = parent; }
  inline void remove_from_parent() { parent_.reset(); }

  virtual void insert_leaves_to(
      std::unordered_set<IdView<SDFTreeNodeId>, IdViewHash<SDFTreeNodeId>, std::equal_to<>>& leaves) = 0;

  virtual void remove_leaves_from(
      std::unordered_set<IdView<SDFTreeNodeId>, IdViewHash<SDFTreeNodeId>, std::equal_to<>>& leaves) = 0;

  virtual void push_dirty_primitives() = 0;

 protected:
  SDFTreeNodeId node_id_;
  TransformId transform_id_;
  Transform transform_;
  SDFBinaryOperation bin_op_;

  std::optional<std::reference_wrapper<GroupNode>> parent_;

  SDFTreeRegistry& tree_registry_;

  std::string name_;
};

template <typename T>
concept SDFTreeNodeConcept = std::is_base_of_v<SDFTreeNode, T>;

}  // namespace resin

#endif
