#ifndef RESIN_SDF_TREE_HPP
#define RESIN_SDF_TREE_HPP

#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/transform.hpp>

namespace resin {

class SDFTreeRegistry {
 public:
  // TODO(migoox): allow specifying the sizes
  SDFTreeRegistry()
      : sphere_component_registry_(IdRegistry<SphereNode>(1000)),
        cube_component_registry_(IdRegistry<CubeNode>(1000)),
        transform_component_registry_(IdRegistry<Transform>(2000)),
        nodes_registry_(IdRegistry<SDFTreeNode>(5000)) {
    all_nodes_.resize(nodes_registry_.get_max_objs());
  }

  // Components: the ids correspond to the uniform buffers indices
  IdRegistry<SphereNode> sphere_component_registry_;
  IdRegistry<CubeNode> cube_component_registry_;
  IdRegistry<Transform> transform_component_registry_;

  // Nodes: the ids correspond to the nodes stored in the array all_nodes_
  IdRegistry<SDFTreeNode> nodes_registry_;

  std::vector<std::optional<std::reference_wrapper<SDFTreeNode>>> all_nodes_;
};

class SDFTree {
 public:
  SDFTree() : root(std::make_unique<GroupNode>(sdf_tree_registry_)) {}

  std::optional<std::reference_wrapper<SDFTreeNode>> get_node(IdView<SDFTreeNodeId> node_id);

 private:
  SDFTreeRegistry sdf_tree_registry_;

 public:
  std::unique_ptr<GroupNode> root;
};

}  // namespace resin

#endif
