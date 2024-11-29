#ifndef RESIN_SDF_TREE_HPP
#define RESIN_SDF_TREE_HPP

#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <libresin/core/transform.hpp>
#include <optional>

namespace resin {

class SDFTreeRegistry {
 public:
  // TODO(migoox): allow specifying the sizes
  SDFTreeRegistry()
      : sphere_component_registry(IdRegistry<SphereNode>(1000)),
        cube_component_registry(IdRegistry<CubeNode>(1000)),
        transform_component_registry(IdRegistry<Transform>(2000)),
        nodes_registry(IdRegistry<SDFTreeNode>(5000)) {
    all_nodes.resize(nodes_registry.get_max_objs());
  }

  // Components: the ids correspond to the uniform buffers indices
  IdRegistry<SphereNode> sphere_component_registry;
  IdRegistry<CubeNode> cube_component_registry;
  IdRegistry<Transform> transform_component_registry;

  // Nodes: the ids correspond to the nodes stored in the array all_nodes_
  IdRegistry<SDFTreeNode> nodes_registry;

  std::vector<std::optional<std::reference_wrapper<SDFTreeNode>>> all_nodes;
};

class SDFTree {
 public:
  SDFTree() : root(std::make_unique<GroupNode>(sdf_tree_registry_)) {}

  bool visit_node(IdView<SDFTreeNodeId> node_id, ISDFTreeNodeVisitor& visitor);

 private:
  SDFTreeRegistry sdf_tree_registry_;

 public:
  std::unique_ptr<GroupNode> root;
};

}  // namespace resin

#endif
