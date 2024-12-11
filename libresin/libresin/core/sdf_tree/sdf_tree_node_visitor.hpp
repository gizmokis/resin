#ifndef RESIN_SDF_TREE_NODE_VISITOR_HPP
#define RESIN_SDF_TREE_NODE_VISITOR_HPP

namespace resin {

class GroupNode;
class BasePrimitiveNode;
class SphereNode;
class CubeNode;

class ISDFTreeNodeVisitor {
 public:
  void virtual visit_sphere(SphereNode&) {}
  void virtual visit_cube(CubeNode&) {}
  void virtual visit_group(GroupNode&) {}
  void virtual visit_primitive(BasePrimitiveNode&) {}

  virtual ~ISDFTreeNodeVisitor() = default;
};

}  // namespace resin

#endif
