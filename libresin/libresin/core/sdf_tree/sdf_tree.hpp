#ifndef RESIN_SDF_TREE_HPP
#define RESIN_SDF_TREE_HPP

#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <libresin/core/transform.hpp>
#include <libresin/utils/exceptions.hpp>
#include <optional>

namespace resin {
template <sdf_shader_consts::SDFShaderPrim PrimType>
class PrimitiveNode;

struct SDFTreeRegistry {
  // TODO(SDF-98): allow specifying the sizes
  SDFTreeRegistry();

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

  // Nodes: the ids correspond to the indices of nodes stored in the array all_nodes_
  IdRegistry<SDFTreeNode> nodes_registry;
  std::vector<std::optional<std::reference_wrapper<SDFTreeNode>>> all_nodes;
  std::vector<std::optional<std::reference_wrapper<GroupNode>>> all_group_nodes;

  std::vector<IdView<SDFTreeNodeId>> dirty_primitives;
};

class SDFTree {
 public:
  SDFTree() : root_(std::make_unique<GroupNode>(sdf_tree_registry_)), tree_id_((curr_id_++)) {}

  std::optional<IdView<SDFTreeNodeId>> get_view_from_raw_id(size_t raw_id);

  void visit_node(IdView<SDFTreeNodeId> node_id, ISDFTreeNodeVisitor& visitor);

  // Visits all dirty primitives AND clears the dirty primitives collection.
  void visit_dirty_primitives(ISDFTreeNodeVisitor& visitor);
  void visit_all_primitives(ISDFTreeNodeVisitor& visitor);

  inline std::vector<IdView<SDFTreeNodeId>> dirty_primitives() const { return sdf_tree_registry_.dirty_primitives; }

  // Cost O(1)
  SDFTreeNode& node(IdView<SDFTreeNodeId> node_id);

  // Cost O(1)
  inline bool is_group(IdView<SDFTreeNodeId> node_id) const {
    return sdf_tree_registry_.all_group_nodes[node_id.raw()].has_value();
  }

  // Cost O(1)
  GroupNode& group(IdView<SDFTreeNodeId> node_id);

  // WARNING: This function must not be called while children of the the provided node's parent are iterated.
  void delete_node(IdView<SDFTreeNodeId> node_id);

  inline std::string gen_shader_code() const { return root_->gen_shader_code(); }

  inline void clear_dirty() { sdf_tree_registry_.dirty_primitives.clear(); }

  inline GroupNode& root() { return *root_; }

  inline size_t tree_id() const { return tree_id_; }

 private:
  static size_t curr_id_;

  SDFTreeRegistry sdf_tree_registry_;
  std::unique_ptr<GroupNode> root_;
  size_t tree_id_;
};

}  // namespace resin

#endif
