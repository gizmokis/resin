#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_base_node.hpp>

namespace resin {

void BasePrimitiveNode::fix_material_ancestors() {
  if (!parent_.has_value()) {
    ancestor_mat_id_ = std::nullopt;
  } else {
    auto ancestor = parent_->get().ancestor_material_id();
    if (ancestor.has_value()) {
      ancestor_mat_id_ = ancestor;
    } else {
      ancestor_mat_id_ = parent_->get().material_id();
    }
  }
}

}  // namespace resin
