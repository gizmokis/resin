#ifndef RESIN_SDF_TREE_NODE_VISITOR_HPP
#define RESIN_SDF_TREE_NODE_VISITOR_HPP

namespace resin {

class SDFTreeNode;
class GroupNode;
class PrimitiveNode;

class ISDFTreeNodeVisitor {
 public:
  virtual void visit_node(SDFTreeNode&) {}
  virtual void visit_group(GroupNode&) {}
  virtual void visit_primitive(PrimitiveNode&) {}

  virtual ~ISDFTreeNodeVisitor() = default;
};

}  // namespace resin

#endif
