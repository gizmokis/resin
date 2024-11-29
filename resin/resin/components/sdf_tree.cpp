#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <libresin/core/sdf/group_node.hpp>
#include <libresin/core/sdf/primitive_node.hpp>
#include <libresin/core/sdf/sdf_tree_node.hpp>
#include <resin/components/sdf_tree.hpp>

namespace resin {

void SDFTreeComponentVisitor::visit_group(GroupNode& node) {
  ::ImGui::PushID(node.node_id().raw_as_int());
  bool tree_node_opened = ::ImGui::TreeNode(node.name().data());
  if (!tree_node_opened) {
    ::ImGui::PopID();
    return;
  }

  for (auto& [child_op, child] : node) {
    child->accept_visitor(*this);
  }

  ::ImGui::TreePop();
  ::ImGui::PopID();
}

void SDFTreeComponentVisitor::visit_primitive(PrimitiveNode& node) {
  ::ImGui::PushID(node.node_id().raw_as_int());
  if (::ImGui::Selectable(node.name().data())) {
    selected_ = node.node_id();
  }
  ::ImGui::PopID();
}

namespace ImGui {  // NOLINT

std::optional<IdView<SDFTreeNodeId>> SDFTree(GroupNode& group_node) {
  SDFTreeComponentVisitor vs;
  group_node.accept_visitor(vs);

  return vs.selected();
}

}  // namespace ImGui

}  // namespace resin
