#ifndef RESIN_SDF_TREE_NODE_VISITOR_HPP
#define RESIN_SDF_TREE_NODE_VISITOR_HPP

namespace resin {

class GroupNode;
class PrimitiveNode;
class SphereNode;
class CubeNode;

class ISDFTreeNodeVisitor {
 public:
  void virtual visit_sphere(SphereNode&) {}
  void virtual visit_cube(CubeNode&) {}
  void virtual visit_group(GroupNode&) {}
  void virtual visit_primitive(PrimitiveNode&) {}

  virtual ~ISDFTreeNodeVisitor() = default;
};

}  // namespace resin

#endif
