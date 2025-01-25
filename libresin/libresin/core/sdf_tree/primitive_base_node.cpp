#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_base_node.hpp>

namespace resin {

void BasePrimitiveNode::fix_material_ancestors() {
  tree_registry_.is_tree_dirty = true;
  if (!parent_.has_value()) {
    ancestor_mat_id_ = std::nullopt;
  } else {
    ancestor_mat_id_ = parent_->get().active_material_id();
  }
}

}  // namespace resin
