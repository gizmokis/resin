#ifndef RESIN_SDF_TREE_NODE_VISITOR_HPP
#define RESIN_SDF_TREE_NODE_VISITOR_HPP
namespace resin {

class SDFTreeNode;
class GroupNode;
class BasePrimitiveNode;
class SphereNode;
class CubeNode;
class TorusNode;
class CapsuleNode;
class LinkNode;
class EllipsoidNode;
class PyramidNode;
class CylinderNode;
class TriangularPrismNode;

class ISDFTreeNodeVisitor {
 public:
  void virtual visit_node(SDFTreeNode&) {}
  void virtual visit_group(GroupNode&) {}
  void virtual visit_primitive(BasePrimitiveNode&) {}
  void virtual visit_sphere(SphereNode&) {}
  void virtual visit_cube(CubeNode&) {}
  void virtual visit_torus(TorusNode&) {}
  void virtual visit_capsule(CapsuleNode&) {}
  void virtual visit_link(LinkNode&) {}
  void virtual visit_ellipsoid(EllipsoidNode&) {}
  void virtual visit_pyramid(PyramidNode&) {}
  void virtual visit_cylinder(CylinderNode&) {}
  void virtual visit_prism(TriangularPrismNode&) {}

  virtual ~ISDFTreeNodeVisitor() = default;
};

}  // namespace resin

#endif
