#ifndef RESIN_SDF_TREE_HPP
#define RESIN_SDF_TREE_HPP

#include <libresin/core/id_registry.hpp>
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

  std::string gen_shader_code(GenShaderMode mode = GenShaderMode::SinglePrimitiveArray) const;

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
