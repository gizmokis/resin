#ifndef RESIN_SDF_TREE_HPP
#define RESIN_SDF_TREE_HPP
#include <libresin/core/id_registry.hpp>
#include <libresin/core/transform.hpp>
#include <memory>

namespace resin {

class SDFTree;
class SDFTreeNode;
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

class SDFTreeRegistry {
 public:
  // TODO(migoox): allow specifying the sizes
  SDFTreeRegistry()
      : sphere_component_registry_(std::make_shared<IdRegistry<SphereNode>>(1000)),
        cube_component_registry_(std::make_shared<IdRegistry<CubeNode>>(1000)),
        transform_component_registry_(std::make_shared<IdRegistry<Transform>>(2000)),
        nodes_registry_(std::make_shared<IdRegistry<SDFTreeNode>>(5000)) {}

 private:
  friend SDFTree;
  friend SDFTreeNode;
  friend GroupNode;
  friend SphereNode;
  friend CubeNode;

  // Components: the ids correspond to the uniform buffers indices
  std::shared_ptr<IdRegistry<SphereNode>> sphere_component_registry_;
  std::shared_ptr<IdRegistry<CubeNode>> cube_component_registry_;
  std::shared_ptr<IdRegistry<Transform>> transform_component_registry_;

  // Nodes: the ids correspond to the nodes stored in the array of all nodes
  std::shared_ptr<IdRegistry<SDFTreeNode>> nodes_registry_;
};

class SDFTree {
 public:
  SDFTree()
      : sdf_tree_registry_(std::make_shared<SDFTreeRegistry>()),
        root(std::make_shared<GroupNode>(sdf_tree_registry_)) {}

 private:
  std::shared_ptr<SDFTreeRegistry> sdf_tree_registry_;

 public:
  std::shared_ptr<GroupNode> root;
};

}  // namespace resin

#endif
