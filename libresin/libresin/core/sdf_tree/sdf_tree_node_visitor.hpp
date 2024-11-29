#ifndef RESIN_SDF_TREE_NODE_VISITOR_HPP
#define RESIN_SDF_TREE_NODE_VISITOR_HPP

namespace resin {

class GroupNode;
class PrimitiveNode;
class SphereNode;
class CubeNode;

class ISDFTreeNodeVisitor {
 public:
  void virtual visit_sphere(SphereNode&)       = 0;
  void virtual visit_cube(CubeNode&)           = 0;
  void virtual visit_group(GroupNode&)         = 0;
  void virtual visit_primitive(PrimitiveNode&) = 0;

  virtual ~ISDFTreeNodeVisitor() = default;
};

}  // namespace resin

#endif
