#ifndef RESIN_SDF_TREE_HPP
#define RESIN_SDF_TREE_HPP

#include <functional>
#include <libresin/core/id_registry.hpp>
#include <libresin/core/material.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <libresin/core/sdf_tree/sdf_tree_registry.hpp>
#include <libresin/core/transform.hpp>
#include <libresin/utils/exceptions.hpp>
#include <optional>

namespace resin {
class GroupNode;

class SDFTree {
 public:
  SDFTree();

  std::optional<IdView<SDFTreeNodeId>> get_view_from_raw_id(size_t raw_id);

  void visit_dirty_primitives(ISDFTreeNodeVisitor& visitor);
  void mark_primitives_clean() { sdf_tree_registry_.dirty_primitives.clear(); }

  void visit_dirty_node_attributes(ISDFTreeNodeVisitor& visitor);
  void mark_node_attributes_clean() { sdf_tree_registry_.dirty_node_attributes.clear(); }

  void visit_all_nodes(ISDFTreeNodeVisitor& visitor);
  void visit_all_primitives(ISDFTreeNodeVisitor& visitor);
  void visit_node(IdView<SDFTreeNodeId> node_id, ISDFTreeNodeVisitor& visitor);

  const SDFTreeRegistry::NodesSet& dirty_primitives() const { return sdf_tree_registry_.dirty_primitives; }
  const SDFTreeRegistry::NodesSet& dirty_node_attributes() const { return sdf_tree_registry_.dirty_node_attributes; }

  // Cost O(1)
  SDFTreeNode& node(IdView<SDFTreeNodeId> node_id);
  const SDFTreeNode& node(IdView<SDFTreeNodeId> node_id) const;

  // Cost O(1)
  bool is_group(IdView<SDFTreeNodeId> node_id) const {
    return sdf_tree_registry_.all_group_nodes[node_id.raw()].has_value();
  }

  // Cost O(1)
  GroupNode& group(IdView<SDFTreeNodeId> node_id);
  const GroupNode& group(IdView<SDFTreeNodeId> node_id) const;

  // WARNING: This function must not be called while children of the the provided node's parent are iterated.
  void delete_node(IdView<SDFTreeNodeId> node_id);

  std::string gen_shader_code(GenShaderMode mode = GenShaderMode::SinglePrimitiveArray) const;

  GroupNode& root() { return *root_; }
  const GroupNode& root() const { return *root_; }

  template <SDFTreeNodeConcept Node, typename... Args>
    requires std::constructible_from<Node, SDFTreeRegistry&, Args...>
  std::unique_ptr<Node> create_detached_node(Args&&... args) {
    return std::make_unique<Node>(sdf_tree_registry_, std::forward<Args>(args)...);
  }

  size_t tree_id() const { return tree_id_; }

  bool is_dirty() const { return sdf_tree_registry_.is_tree_dirty; }
  void mark_clean() { sdf_tree_registry_.is_tree_dirty = false; }

  MaterialSDFTreeComponent& material(IdView<MaterialId> mat_id);
  const MaterialSDFTreeComponent& material(IdView<MaterialId> mat_id) const;
  MaterialSDFTreeComponent& add_material(Material mat);

  // Cost: O(nm), where n is a number of nodes and m is a number of materials.
  // Note: Throws if the `mat_id` is the default material id.
  void delete_material(IdView<MaterialId> mat_id);

  // Note: The vector does not contain the default material.
  const std::vector<IdView<MaterialId>>& materials() const { return material_active_ids_; }

  MaterialSDFTreeComponent& default_material() { return sdf_tree_registry_.default_material; }
  const MaterialSDFTreeComponent& default_material() const { return sdf_tree_registry_.default_material; }

  // Visits all materials including the default material.
  void visit_all_materials(const std::function<void(MaterialSDFTreeComponent&)>& mat_visitor);

  void visit_dirty_materials(const std::function<void(MaterialSDFTreeComponent&)>& mat_visitor);
  void mark_materials_clean() { sdf_tree_registry_.dirty_materials.clear(); }

  size_t max_node_count() const { return sdf_tree_registry_.nodes_registry.get_max_objs(); }
  size_t max_material_count() const { return sdf_tree_registry_.materials_registry.get_max_objs(); }

  void set_root(std::unique_ptr<GroupNode> root);
  void clear();

 private:
  static size_t curr_id_;

  SDFTreeRegistry sdf_tree_registry_;
  std::unique_ptr<GroupNode> root_;
  size_t tree_id_;

  std::vector<IdView<MaterialId>> material_active_ids_;
  std::vector<std::optional<std::unique_ptr<MaterialSDFTreeComponent>>> materials_;
};

}  // namespace resin

#endif
