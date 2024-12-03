#ifndef RESIN_SDF_TREE_HPP
#define RESIN_SDF_TREE_HPP

#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <libresin/core/transform.hpp>
#include <libresin/utils/exceptions.hpp>
#include <optional>

namespace resin {

struct SDFTreeRegistry {
  // TODO(migoox): allow specifying the sizes
  SDFTreeRegistry()
      : sphere_component_registry(IdRegistry<SphereNode>(1000)),
        cube_component_registry(IdRegistry<CubeNode>(1000)),
        transform_component_registry(IdRegistry<Transform>(2000)),
        nodes_registry(IdRegistry<SDFTreeNode>(5000)) {
    all_nodes.resize(nodes_registry.get_max_objs());
    all_nodes.resize(nodes_registry.get_max_objs());
    dirty_primitives.reserve(nodes_registry.get_max_objs());
  }

  // Components: the ids correspond to the uniform buffers indices
  IdRegistry<SphereNode> sphere_component_registry;
  IdRegistry<CubeNode> cube_component_registry;
  IdRegistry<Transform> transform_component_registry;

  // Nodes: the ids correspond to the indices of nodes stored in the array all_nodes_
  IdRegistry<SDFTreeNode> nodes_registry;
  std::vector<std::optional<std::reference_wrapper<SDFTreeNode>>> all_nodes;
  std::vector<std::optional<std::reference_wrapper<GroupNode>>> all_group_nodes;

  std::vector<IdView<SDFTreeNodeId>> dirty_primitives;
};

class SDFTree {
 public:
  SDFTree() : root_(std::make_unique<GroupNode>(sdf_tree_registry_)) {}

  std::optional<IdView<SDFTreeNodeId>> get_view_from_raw_id(size_t raw_id);

  void visit_node(IdView<SDFTreeNodeId> node_id, ISDFTreeNodeVisitor& visitor);

  // Visits all dirty primitives AND clears the dirty primitives collection.
  void visit_dirty_primitives(ISDFTreeNodeVisitor& visitor);
  void visit_all_primitives(ISDFTreeNodeVisitor& visitor);

  SDFTreeNode& node(IdView<SDFTreeNodeId> node_id);
  GroupNode& group(IdView<SDFTreeNodeId> node_id);

  // WARNING: This function must not be called while children of the the provided node's parent are iterated.
  void delete_node(IdView<SDFTreeNodeId> node_id);

  inline std::string gen_shader_code() const { return root_->gen_shader_code(); }

  inline void clear_dirty() { sdf_tree_registry_.dirty_primitives.clear(); }

  inline GroupNode& root() { return *root_; }

 private:
  SDFTreeRegistry sdf_tree_registry_;
  std::unique_ptr<GroupNode> root_;
};

}  // namespace resin

#endif
