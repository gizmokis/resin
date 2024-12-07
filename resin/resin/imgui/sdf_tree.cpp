#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/utils/logger.hpp>
#include <optional>
#include <resin/imgui/sdf_tree.hpp>

namespace ImGui {  // NOLINT

namespace resin {

void SDFTreeComponentVisitor::drag_and_drop(::resin::SDFTreeNode& node) {
  auto curr_id = node.node_id();
  if (ImGui::BeginDragDropTarget()) {
    ImVec2 item_min = ImGui::GetItemRectMin();
    ImVec2 item_max = ImGui::GetItemRectMax();
    bool below      = ImGui::GetItemRectSize().y / 2.F > item_max.y - ImGui::GetMousePos().y;

    if (below) {
      ImGui::GetForegroundDrawList()->AddLine(ImVec2(item_min.x, item_max.y), item_max, IM_COL32(255, 255, 0, 255));
    } else {
      ImGui::GetForegroundDrawList()->AddLine(item_min, ImVec2(item_max.x, item_min.y), IM_COL32(255, 255, 0, 255));
    }

    if (const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload("SDF_TREE_DND_PAYLOAD", ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
      IM_ASSERT(payload->DataSize == sizeof(::resin::IdView<::resin::SDFTreeNodeId>));
      ::resin::IdView<::resin::SDFTreeNodeId> source_id =
          *static_cast<const ::resin::IdView<::resin::SDFTreeNodeId>*>(payload->Data);

      move_source_target_ = source_id;
      if (below) {
        move_after_target_ = node.node_id();
      } else {
        move_before_target_ = node.node_id();
      }
    }

    ImGui::EndDragDropTarget();
  }

  if (ImGui::BeginDragDropSource()) {
    ImGui::SetDragDropPayload("SDF_TREE_DND_PAYLOAD", &curr_id, sizeof(::resin::IdView<::resin::SDFTreeNodeId>));
    ImGui::Text("%s", node.name().data());
    ImGui::EndDragDropSource();
  }
}

void SDFTreeComponentVisitor::visit_group(::resin::GroupNode& node) {
  static ImGuiTreeNodeFlags base_flags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

  auto tree_flags       = base_flags;
  bool is_node_selected = false;

  if (selected_ == node.node_id() || is_parent_selected_) {
    tree_flags |= ImGuiTreeNodeFlags_Selected;
    is_node_selected = true;
  }

  ImGui::PushID(static_cast<int>(node.node_id().raw()));
  bool tree_node_opened = ImGui::TreeNodeEx(node.name().data(), tree_flags);
  if (ImGui::IsItemClicked()) {
    is_node_selected = true;
    selected_        = node.node_id();
  }

  if (node.has_parent()) {
    drag_and_drop(node);
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

void SDFTreeComponentVisitor::visit_primitive(::resin::PrimitiveNode& node) {
  bool is_selected = false;

  is_selected = selected_ == node.node_id() || is_parent_selected_;

  ImGui::PushID(static_cast<int>(node.node_id().raw()));
  if (ImGui::Selectable(node.name().data(), &is_selected)) {
    selected_ = node.node_id();
  }

  drag_and_drop(node);

  ImGui::PopID();
}

void SDFTreeComponentVisitor::apply_move_operation(::resin::SDFTree& tree) {
  if (!move_source_target_.has_value()) {
    return;
  }
  auto node_ptr = tree.node(*move_source_target_).parent().detach_child(*move_source_target_);

  if (move_before_target_.has_value()) {
    tree.node(*move_before_target_).parent().insert_before_child(*move_before_target_, std::move(node_ptr));
  }

  if (move_after_target_.has_value()) {
    tree.node(*move_after_target_).parent().insert_after_child(*move_after_target_, std::move(node_ptr));
  }
}

void SDFTreeOperationVisitor::visit_group(::resin::GroupNode& node) {
  if (op == SDFTreeOperationVisitor::Operation::PushGroup) {
    node.push_back_child<::resin::GroupNode>(::resin::SDFBinaryOperation::Union)
        .push_back_child<::resin::SphereNode>(::resin::SDFBinaryOperation::Union);
  } else if (op == SDFTreeOperationVisitor::Operation::PushPrimitive) {
    node.push_back_child<::resin::SphereNode>(::resin::SDFBinaryOperation::Union);
  }
}

void SDFTreeOperationVisitor::visit_primitive(::resin::PrimitiveNode& node) {
  if (op == SDFTreeOperationVisitor::Operation::PushGroup) {
    node.parent()
        .push_back_child<::resin::GroupNode>(::resin::SDFBinaryOperation::Union)
        .push_back_child<::resin::SphereNode>(::resin::SDFBinaryOperation::Union);
  } else if (op == SDFTreeOperationVisitor::Operation::PushPrimitive) {
    node.parent().push_back_child<::resin::SphereNode>(::resin::SDFBinaryOperation::Union);
  }
}

std::optional<::resin::IdView<::resin::SDFTreeNodeId>> SDFTreeView(::resin::SDFTree& tree) {
  static std::optional<::resin::IdView<::resin::SDFTreeNodeId>> selected = std::nullopt;

  auto comp_vs = resin::SDFTreeComponentVisitor(selected);
  resin::SDFTreeOperationVisitor op_vs;

  if (ImGui::BeginChild("ResizableChild", ImVec2(-FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * 8),
                        ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY)) {
    tree.root().accept_visitor(comp_vs);
    selected = comp_vs.selected();
  }
  ImGui::EndChild();

  comp_vs.apply_move_operation(tree);

  ImGui::IsItemClicked();
  ImGui::GetMouseDragDelta();

  bool delete_disabled = !selected.has_value() || !tree.node(selected.value()).has_parent() ||
                         (tree.node(selected.value()).parent().get_children_count() == 1 &&
                          tree.node(selected.value()).parent().node_id() == tree.root().node_id());
  if (delete_disabled) {
    ImGui::BeginDisabled();
  }

  if (ImGui::Button("Delete")) {
    if (tree.node(selected.value()).has_parent() && tree.node(selected.value()).parent().get_children_count() == 1) {
      tree.delete_node(tree.node(selected.value()).parent().node_id());
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
      tree.node(comp_vs.selected().value()).accept_visitor(op_vs);
    }
  }

  ImGui::SameLine();
  if (ImGui::Button("Push group")) {
    if (comp_vs.selected().has_value()) {
      op_vs.op = SDFTreeOperationVisitor::Operation::PushGroup;
      tree.node(comp_vs.selected().value()).accept_visitor(op_vs);
    }
  }

  if (push_disabled) {
    ImGui::EndDisabled();
  }

  return selected;
}

}  // namespace resin

}  // namespace ImGui
