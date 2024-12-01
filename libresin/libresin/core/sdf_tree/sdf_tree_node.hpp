#ifndef RESIN_SDF_TREE_NODE_HPP
#define RESIN_SDF_TREE_NODE_HPP

#include <functional>
#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <libresin/core/transform.hpp>
#include <string>

namespace resin {

using SDFBinaryOperation = sdf_shader_consts::SDFShaderBinOp;

class SDFTreeRegistry;
class SDFTree;

class SDFTreeNode;
using SDFTreeNodeId = Id<SDFTreeNode>;

class SDFTreeNode {
 public:
  virtual std::string gen_shader_code() const               = 0;
  virtual void accept_visitor(ISDFTreeNodeVisitor& visitor) = 0;
  virtual std::string_view name() const                     = 0;
  virtual void rename(std::string&&)                        = 0;

  SDFTreeNode() = delete;

  inline IdView<SDFTreeNodeId> node_id() const { return node_id_; }
  inline IdView<TransformId> transform_component_id() const { return transform_id_; }
  inline Transform& transform() { return transform_; }

  // TODO(migoox): add material component

  inline SDFBinaryOperation bin_op() const { return bin_op_; }
  inline void set_bin_op(SDFBinaryOperation bin_op) { bin_op_ = bin_op; }

  // It is the programmer's responsibility to assert that SDFTreeNode class will not outlive the provided registry!
  explicit SDFTreeNode(SDFTreeRegistry& tree);

  virtual ~SDFTreeNode();

  inline bool has_parent() const { return this->parent_.has_value(); }
  inline GroupNode& parent() { return this->parent_.value(); }
  inline const GroupNode& parent() const { return this->parent_.value(); }

  bool has_neighbor_up() const;
  SDFTreeNode& neighbor_up();
  SDFTreeNode& neighbor_up() const;

  bool has_neighbor_down() const;
  SDFTreeNode& neighbor_down();
  SDFTreeNode& neighbor_down() const;

 protected:
  friend SDFTree;
  friend GroupNode;

  inline void set_parent(GroupNode& parent) { this->parent_ = parent; }

 protected:
  SDFTreeNodeId node_id_;
  TransformId transform_id_;
  Transform transform_;
  SDFBinaryOperation bin_op_;

  std::optional<std::reference_wrapper<GroupNode>> parent_;

  std::reference_wrapper<SDFTreeRegistry> tree_registry_;
};

template <typename T>
concept SDFTreeNodeConcept = std::is_base_of_v<SDFTreeNode, T>;

}  // namespace resin

#endif
