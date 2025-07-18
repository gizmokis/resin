#ifndef RESIN_SDF_TREE_REGISTRY_HPP
#define RESIN_SDF_TREE_REGISTRY_HPP

#include <libresin/core/id_registry.hpp>
#include <libresin/core/material.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/sdf_primitive_type_manager.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/transform.hpp>
#include <optional>

namespace resin {

struct SDFTreeRegistry {
  using NodesSet     = std::unordered_set<IdView<SDFTreeNodeId>, IdViewHash<SDFTreeNodeId>, std::equal_to<>>;
  using MaterialsSet = std::unordered_set<IdView<MaterialId>, IdViewHash<MaterialId>, std::equal_to<>>;

  // TODO(SDF-98): allow specifying the sizes
  SDFTreeRegistry()
      : transform_component_registry(IdRegistry<Transform>(100)),
        primitives_registry(IdRegistry<PrimitiveNode>(100)),
        nodes_registry(IdRegistry<SDFTreeNode>(100)),
        materials_registry(IdRegistry<Material>(100)),
        default_material(*this) {
    all_nodes.resize(nodes_registry.get_max_objs());
    all_group_nodes.resize(nodes_registry.get_max_objs());
    all_primitive_nodes.resize(nodes_registry.get_max_objs());
  }

  IdRegistry<Transform> transform_component_registry;
  IdRegistry<PrimitiveNode> primitives_registry;

  // Nodes: the ids correspond to the indices of nodes stored in the array all_nodes_
  IdRegistry<SDFTreeNode> nodes_registry;
  std::vector<std::optional<std::reference_wrapper<SDFTreeNode>>> all_nodes;
  std::vector<std::optional<std::reference_wrapper<GroupNode>>> all_group_nodes;
  std::vector<std::optional<std::reference_wrapper<PrimitiveNode>>> all_primitive_nodes;

  NodesSet dirty_primitives;
  NodesSet dirty_node_attributes;

  IdRegistry<Material> materials_registry;
  MaterialsSet dirty_materials;

  // Required for shader generation
  MaterialSDFTreeComponent default_material;

  // Note: If the type manager gets cleared, all of the tree nodes falls into empty state as they point to
  // non-existing types.
  SDFPrimitiveTypeManager primitive_type_manager_{};

  size_t node_index{};
  size_t material_index{};

  bool is_tree_dirty{true};
};

}  // namespace resin

#endif
