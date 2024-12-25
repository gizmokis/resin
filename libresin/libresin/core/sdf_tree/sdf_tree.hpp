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

  // Visits all dirty primitives AND clears the dirty primitives collection.
  void visit_dirty_primitives(ISDFTreeNodeVisitor& visitor);
  inline void clear_dirty_primitives() { sdf_tree_registry_.dirty_primitives.clear(); }

  void visit_all_primitives(ISDFTreeNodeVisitor& visitor);
  void visit_node(IdView<SDFTreeNodeId> node_id, ISDFTreeNodeVisitor& visitor);

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

  std::string gen_shader_code() const;

  inline GroupNode& root() { return *root_; }
  inline const GroupNode& root() const { return *root_; }

  template <SDFTreeNodeConcept Node, typename... Args>
    requires std::constructible_from<Node, SDFTreeRegistry&, Args...>
  std::unique_ptr<Node> create_detached_node(Args&&... args) {
    return std::make_unique<Node>(sdf_tree_registry_, std::forward<Args>(args)...);
  }

  inline size_t tree_id() const { return tree_id_; }

  MaterialSDFTreeComponent& material(IdView<MaterialId> mat_id);
  MaterialSDFTreeComponent& add_material(Material mat);

  // Cost: O(nm), where n is a number of nodes and m is a number of materials.
  // Note: Throws if the `mat_id` is the default material id.
  void delete_material(IdView<MaterialId> mat_id);

  inline void mark_material_dirty(IdView<MaterialId> mat_id) { sdf_tree_registry_.dirty_materials.push_back(mat_id); }

  // Note: The vector does not contain the default material.
  inline const std::vector<IdView<MaterialId>>& materials() const { return material_active_ids_; }

  inline MaterialSDFTreeComponent& default_material() { return sdf_tree_registry_.default_material; }

  // Visits all dirty materials AND clears the dirty materials collection.
  void visit_dirty_materials(const std::function<void(MaterialSDFTreeComponent&)>& mat_visitor);
  inline void clear_dirty_materials() { sdf_tree_registry_.dirty_materials.clear(); }

 private:
  static size_t curr_id_;

  SDFTreeRegistry sdf_tree_registry_;
  std::unique_ptr<GroupNode> root_;
  size_t tree_id_;

  std::vector<IdView<MaterialId>> material_active_ids_;
  std::vector<std::optional<MaterialSDFTreeComponent>> materials_;
};

}  // namespace resin

#endif
