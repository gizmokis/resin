#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <optional>
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
    selected_ = node.node_id().raw();
  }
  ::ImGui::PopID();
}

namespace ImGui {  // NOLINT

std::optional<size_t> SDFTreeView(SDFTree& tree) {
  SDFTreeComponentVisitor vs;
  tree.root->accept_visitor(vs);

  return vs.selected();
  return std::nullopt;
}

}  // namespace ImGui

}  // namespace resin
