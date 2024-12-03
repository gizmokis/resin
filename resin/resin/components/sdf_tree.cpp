#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <optional>
#include <resin/components/sdf_tree.hpp>

namespace resin {

void SDFTreeComponentVisitor::visit_group(GroupNode& node) {
  static ImGuiTreeNodeFlags base_flags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

  auto tree_flags       = base_flags;
  bool is_node_selected = false;

  if (selected_ == node.node_id() || is_parent_selected_) {
    tree_flags |= ImGuiTreeNodeFlags_Selected;
    is_node_selected = true;
  }

  ImGui::PushID(node.node_id().raw_as_int());
  bool tree_node_opened = ImGui::TreeNodeEx(node.name().data(), tree_flags);
  if (ImGui::IsItemClicked()) {
    is_node_selected = true;
    selected_        = node.node_id();
  }

  if (!tree_node_opened) {
    ImGui::PopID();
    return;
  }

  for (auto child_it = node.begin(); child_it != node.end(); ++child_it) {
    is_parent_selected_ = is_node_selected;
    node.get_child(*child_it).accept_visitor(*this);
    is_parent_selected_ = is_node_selected;
  }

  ImGui::TreePop();
  ImGui::PopID();
}

void SDFTreeComponentVisitor::visit_primitive(PrimitiveNode& node) {
  bool is_selected = false;

  is_selected = selected_ == node.node_id() || is_parent_selected_;

  ImGui::PushID(node.node_id().raw_as_int());
  if (ImGui::Selectable(node.name().data(), &is_selected)) {
    selected_ = node.node_id();
  }
  ImGui::PopID();
}

void SDFTreeOperationVisitor::visit_group(GroupNode& node) {
  if (op == SDFTreeOperationVisitor::Operation::PushGroup) {
    node.push_back_child<GroupNode>(SDFBinaryOperation::Union);
  } else if (op == SDFTreeOperationVisitor::Operation::PushPrimitive) {
    node.push_back_child<SphereNode>(SDFBinaryOperation::Union);
  }
}

void SDFTreeOperationVisitor::visit_primitive(PrimitiveNode& node) {
  if (op == SDFTreeOperationVisitor::Operation::PushGroup) {
    node.parent().push_back_child<GroupNode>(SDFBinaryOperation::Union);
  } else if (op == SDFTreeOperationVisitor::Operation::PushPrimitive) {
    node.parent().push_back_child<SphereNode>(SDFBinaryOperation::Union);
  }
}

}  // namespace resin

namespace ImGui {  // NOLINT

std::optional<resin::IdView<resin::SDFTreeNodeId>> SDFTreeView(resin::SDFTree& tree) {
  static std::optional<resin::IdView<resin::SDFTreeNodeId>> selected = std::nullopt;

  auto comp_vs = resin::SDFTreeComponentVisitor(selected);
  resin::SDFTreeOperationVisitor op_vs;

  if (ImGui::BeginChild("ResizableChild", ImVec2(-FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * 8),
                        ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY)) {
    tree.root().accept_visitor(comp_vs);
    selected = comp_vs.selected();
  }
  ImGui::EndChild();

  bool delete_disabled = !selected.has_value() || !tree.get_node(selected.value()).has_parent() ||
                         (tree.get_node(selected.value()).parent().get_children_count() == 1 &&
                          tree.get_node(selected.value()).parent().node_id() == tree.root().node_id());
  if (delete_disabled) {
    ImGui::BeginDisabled();
  }

  if (ImGui::Button("Delete")) {
    if (tree.get_node(selected.value()).has_parent() &&
        tree.get_node(selected.value()).parent().get_children_count() == 1) {
      tree.delete_node(tree.get_node(selected.value()).parent().node_id());
    } else {
      tree.delete_node(selected.value());
    }
    selected = std::nullopt;
  }

  if (delete_disabled) {
    ImGui::EndDisabled();
  }

  ImGui::SameLine();

  bool push_disabled = !selected.has_value();
  if (push_disabled) {
    ImGui::BeginDisabled();
  }

  if (ImGui::Button("Push sphere")) {
    if (comp_vs.selected().has_value()) {
      op_vs.op = resin::SDFTreeOperationVisitor::Operation::PushPrimitive;
      tree.get_node(comp_vs.selected().value()).accept_visitor(op_vs);
    }
  }

  ImGui::SameLine();
  if (ImGui::Button("Push group")) {
    if (comp_vs.selected().has_value()) {
      op_vs.op = resin::SDFTreeOperationVisitor::Operation::PushGroup;
      tree.get_node(comp_vs.selected().value()).accept_visitor(op_vs);
    }
  }

  if (push_disabled) {
    ImGui::EndDisabled();
  }

  return selected;
}

}  // namespace ImGui
