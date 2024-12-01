#ifndef RESIN_SDF_TREE_HPP
#define RESIN_SDF_TREE_HPP

#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <libresin/core/transform.hpp>
#include <optional>

#include "libresin/utils/exceptions.hpp"

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

  // Nodes: the ids correspond to the indices of nodes stored in the array all_nodes_
  IdRegistry<SDFTreeNode> nodes_registry;

  std::vector<std::optional<std::reference_wrapper<SDFTreeNode>>> all_nodes;
};

class SDFTree {
 public:
  SDFTree() : root(std::make_unique<GroupNode>(sdf_tree_registry_)) {}

  std::optional<IdView<SDFTreeNodeId>> get_view_from_raw_id(size_t raw_id);

  // Cost: O(1)
  void visit_node(IdView<SDFTreeNodeId> node_id, ISDFTreeNodeVisitor& visitor);

  // Cost: O(1)
  void visit_dirty_primitives(ISDFTreeNodeVisitor& visitor);

  // Cost: O(1)
  SDFTreeNode& get_node(IdView<SDFTreeNodeId> node_id);

  // Cost: Amortized O(1)
  // WARNING: This function must not be called while children of the the provided node's parent are iterated.
  void delete_node(IdView<SDFTreeNodeId> node_id);

  // Cost: O(n)
  std::string gen_shader_code() const { return this->root->gen_shader_code(); }

 private:
  SDFTreeRegistry sdf_tree_registry_;

 public:
  std::unique_ptr<GroupNode> root;
};

}  // namespace resin

#endif
