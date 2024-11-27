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

  ~GroupNode() override = default;

  void push_node(SDFBinaryOperation op, std::weak_ptr<const SDFTreeNode> node);

  void set_op(int node_id, SDFBinaryOperation op);
  void set_obj(int node_id, std::weak_ptr<const SDFTreeNode>);
  void reorder(std::vector<int> id_order);
  void remove_node(int node_id);

  inline void accept_visitor(IMutableSDFTreeNodeVisitor& visitor) override { visitor.visit_group(*this); }
  inline void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) override { visitor.visit_group(*this); }

 private:
  void remove_expired() const;

 private:
  mutable std::vector<std::pair<SDFBinaryOperation, std::weak_ptr<const SDFTreeNode>>> nodes_;
};

}  // namespace resin

#endif
