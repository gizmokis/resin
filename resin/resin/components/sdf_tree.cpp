#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <optional>
#include <resin/components/sdf_tree.hpp>

namespace resin {

void SDFTreeComponentVisitor::visit_group(GroupNode& node) {
  level_++;
  ::ImGui::PushID(node.node_id().raw_as_int());
  bool tree_node_opened = ::ImGui::TreeNode(node.name().data());

  ::ImGui::SameLine();
  if (::ImGui::Button("Push sphere")) {
    node.push_child<SphereNode>(SDFBinaryOperation::Union);
  }
  ::ImGui::SameLine();
  if (::ImGui::Button("Push group")) {
    node.push_child<GroupNode>(SDFBinaryOperation::Union);
  }

  if (!tree_node_opened) {
    ::ImGui::PopID();
    return;
  }

  for (auto child_it = node.begin(); child_it != node.end(); ++child_it) {
    curr_children_count_ = node.get_children_count();
    node.get_child(*child_it)->accept_visitor(*this);
    if (delete_) {
      child_it = node.erase_child(child_it);
      delete_  = false;
    }
  }

  if (node.get_children_count() == 0) {
    delete_ = true;
  }

  ::ImGui::TreePop();
  ::ImGui::PopID();
  level_--;
}

void SDFTreeComponentVisitor::visit_primitive(PrimitiveNode& node) {
  ::ImGui::PushID(node.node_id().raw_as_int());
  if (::ImGui::Selectable(node.name().data())) {
    selected_ = node.node_id().raw();
  }
  ::ImGui::SameLine();
  if (level_ != 1 || curr_children_count_ != 1) {
    if (::ImGui::Button("Delete")) {
      delete_ = true;
    }
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
