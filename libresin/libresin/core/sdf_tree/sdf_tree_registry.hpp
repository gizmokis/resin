#ifndef RESIN_SDF_TREE_REGISTRY_HPP
#define RESIN_SDF_TREE_REGISTRY_HPP

#include <libresin/core/id_registry.hpp>
#include <libresin/core/material.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/transform.hpp>
#include <optional>

namespace resin {
template <sdf_shader_consts::SDFShaderPrim PrimType>
class PrimitiveNode;

struct SDFTreeRegistry {
  // TODO(SDF-98): allow specifying the sizes
  SDFTreeRegistry()
      : sphere_components_registry(IdRegistry<PrimitiveNode<sdf_shader_consts::SDFShaderPrim::Sphere>>(1000)),
        cubes_components_registry(IdRegistry<PrimitiveNode<sdf_shader_consts::SDFShaderPrim::Cube>>(1000)),
        transform_component_registry(IdRegistry<Transform>(2000)),
        primitives_registry(IdRegistry<BasePrimitiveNode>(2000)),
        nodes_registry(IdRegistry<SDFTreeNode>(5000)),
        materials_registry(IdRegistry<Material>(2000)),
        default_material(*this) {
    all_nodes.resize(nodes_registry.get_max_objs());
    all_group_nodes.resize(nodes_registry.get_max_objs());
    dirty_primitives.reserve(nodes_registry.get_max_objs());
  }

  IdRegistry<PrimitiveNode<sdf_shader_consts::SDFShaderPrim::Sphere>> sphere_components_registry;
  IdRegistry<PrimitiveNode<sdf_shader_consts::SDFShaderPrim::Cube>> cubes_components_registry;

  template <sdf_shader_consts::SDFShaderPrim EnumType>
  constexpr IdRegistry<PrimitiveNode<EnumType>>& primitive_components_registry() {
    if constexpr (sdf_shader_consts::SDFShaderPrim::Sphere == EnumType) {
      return sphere_components_registry;
    } else if constexpr (sdf_shader_consts::SDFShaderPrim::Cube == EnumType) {
      return cubes_components_registry;
    } else {
      static_assert(false, "Unsupported registry");
    }
  }

  IdRegistry<Transform> transform_component_registry;
  IdRegistry<BasePrimitiveNode> primitives_registry;

  // Nodes: the ids correspond to the indices of nodes stored in the array all_nodes_
  IdRegistry<SDFTreeNode> nodes_registry;
  std::vector<std::optional<std::reference_wrapper<SDFTreeNode>>> all_nodes;
  std::vector<std::optional<std::reference_wrapper<GroupNode>>> all_group_nodes;

  std::vector<IdView<SDFTreeNodeId>> dirty_primitives;

  IdRegistry<Material> materials_registry;
  std::vector<IdView<MaterialId>> dirty_materials;

  MaterialSDFTreeComponent default_material;

  size_t node_index{};
  size_t material_index{};
};

}  // namespace resin

#endif
