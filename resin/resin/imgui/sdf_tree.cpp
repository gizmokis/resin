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

std::optional<::resin::IdView<::resin::SDFTreeNodeId>> SDFTreeComponentVisitor::get_curr_payload() {
  std::optional<::resin::IdView<::resin::SDFTreeNodeId>> source_id;
  if (const ImGuiPayload* payload = ImGui::GetDragDropPayload()) {
    if (payload->IsDataType(payload_type_.c_str())) {
      IM_ASSERT(payload->DataSize == sizeof(::resin::IdView<::resin::SDFTreeNodeId>));
      source_id = *static_cast<const ::resin::IdView<::resin::SDFTreeNodeId>*>(payload->Data);
    }
  }

  return source_id;
}

void SDFTreeComponentVisitor::drag_and_drop(::resin::SDFTreeNode& node, bool ignore_middle) {
  auto curr_id = node.node_id();
  if (ImGui::BeginDragDropSource()) {
    ImGui::SetDragDropPayload(payload_type_.c_str(), &curr_id, sizeof(::resin::IdView<::resin::SDFTreeNodeId>));
    ImGui::Text("%s", node.name().data());
    ImGui::EndDragDropSource();
  }

  if (ImGui::BeginDragDropTarget()) {
    ImVec2 item_min = ImGui::GetItemRectMin();
    ImVec2 item_max = ImGui::GetItemRectMax();

    float bottom_dist = item_max.y - ImGui::GetMousePos().y;
    bool middle       = !ignore_middle && ImGui::GetItemRectSize().y / 3.F * 2.F > bottom_dist &&
                  ImGui::GetItemRectSize().y / 3.F < bottom_dist;
    bool below = ImGui::GetItemRectSize().y / 2.F > bottom_dist;

    if (middle) {
      ImGui::GetForegroundDrawList()->AddRect(item_min, item_max, IM_COL32(255, 255, 0, 255));
    } else if (below) {
      ImGui::GetForegroundDrawList()->AddLine(ImVec2(item_min.x, item_max.y), item_max, IM_COL32(255, 255, 0, 255));
    } else {
      ImGui::GetForegroundDrawList()->AddLine(item_min, ImVec2(item_max.x, item_min.y), IM_COL32(255, 255, 0, 255));
    }

    if (const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload(payload_type_.c_str(), ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
      IM_ASSERT(payload->DataSize == sizeof(::resin::IdView<::resin::SDFTreeNodeId>));
      auto source_id = *static_cast<const ::resin::IdView<::resin::SDFTreeNodeId>*>(payload->Data);

      move_source_target_ = source_id;
      if (middle) {
        move_into_target_ = node.node_id();
      } else if (below) {
        move_after_target_ = node.node_id();
      } else {
        move_before_target_ = node.node_id();
      }
    }

    ImGui::EndDragDropTarget();
  }
}

void SDFTreeComponentVisitor::visit_group(::resin::GroupNode& node) {
  static ImGuiTreeNodeFlags base_flags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

  auto tree_flags = base_flags;

  if (node.is_leaf()) {
    tree_flags |= ImGuiTreeNodeFlags_Leaf;
  }

  bool is_node_dragged = is_parent_dragged_;
  if (!is_node_dragged) {
    auto source_id  = get_curr_payload();
    is_node_dragged = source_id.has_value() && *source_id == node.node_id();
  }

  bool is_node_selected = false;
  if (selected_ == node.node_id() || is_parent_selected_) {
    tree_flags |= ImGuiTreeNodeFlags_Selected;
    is_node_selected = true;
  }

  ImGui::PushID(static_cast<int>(node.node_id().raw()));

  if (is_node_dragged) {
    ImGui::BeginDisabled();
  }

  bool tree_node_opened = ImGui::TreeNodeEx(node.name().data(), tree_flags);

  if (is_node_dragged) {
    ImGui::EndDisabled();
  }

  if (ImGui::IsItemClicked()) {
    is_node_selected = true;
    selected_        = node.node_id();
  }

  if (!is_node_dragged) {
    drag_and_drop(node, false);
  }

  if (!tree_node_opened) {
    ImGui::PopID();
    return;
  }

  for (auto child_it = node.begin(); child_it != node.end(); ++child_it) {
    is_parent_selected_ = is_node_selected;
    is_parent_dragged_  = is_node_dragged;
    node.get_child(*child_it).accept_visitor(*this);
  }

  ImGui::TreePop();
  ImGui::PopID();
}

void SDFTreeComponentVisitor::visit_primitive(::resin::PrimitiveNode& node) {
  auto source_id = get_curr_payload();

  bool is_node_selected = is_parent_selected_ || selected_ == node.node_id();
  bool is_node_dragged  = is_parent_dragged_ || (source_id.has_value() && *source_id == node.node_id());

  ImGui::PushID(static_cast<int>(node.node_id().raw()));
  if (is_node_dragged) {
    ImGui::BeginDisabled();
  }

  if (ImGui::Selectable(node.name().data(), &is_node_selected)) {
    selected_ = node.node_id();
  }

  if (is_node_dragged) {
    ImGui::EndDisabled();
  }

  if (!is_node_dragged) {
    drag_and_drop(node, true);
  }

  ImGui::PopID();
}

void SDFTreeComponentVisitor::visit_root(::resin::GroupNode& node) {
  for (auto child_it = node.begin(); child_it != node.end(); ++child_it) {
    is_parent_selected_ = false;
    is_parent_dragged_  = false;
    node.get_child(*child_it).accept_visitor(*this);
  }
}

void SDFTreeComponentVisitor::apply_move_operation(::resin::SDFTree& tree) {
  if (!move_source_target_.has_value()) {
    return;
  }

  if (move_into_target_.has_value() && tree.node(*move_source_target_).parent().node_id() != *move_into_target_) {
    auto node_ptr = tree.node(*move_source_target_).parent().detach_child(*move_source_target_);
    tree.group(*move_into_target_).push_back_child(std::move(node_ptr));
  } else if (move_before_target_.has_value()) {
    auto node_ptr = tree.node(*move_source_target_).parent().detach_child(*move_source_target_);
    tree.node(*move_before_target_).parent().insert_before_child(*move_before_target_, std::move(node_ptr));
  } else if (move_after_target_.has_value()) {
    auto node_ptr = tree.node(*move_source_target_).parent().detach_child(*move_source_target_);
    tree.node(*move_after_target_).parent().insert_after_child(*move_after_target_, std::move(node_ptr));
  }
}

void SDFTreeOperationVisitor::visit_group(::resin::GroupNode& node) {
  if (op == SDFTreeOperationVisitor::Operation::PushGroup) {
    node.push_back_child<::resin::GroupNode>(::resin::SDFBinaryOperation::Union);
  } else if (op == SDFTreeOperationVisitor::Operation::PushPrimitive) {
    node.push_back_child<::resin::SphereNode>(::resin::SDFBinaryOperation::Union);
  }
}

void SDFTreeOperationVisitor::visit_primitive(::resin::PrimitiveNode& node) {
  if (op == SDFTreeOperationVisitor::Operation::PushGroup) {
    node.parent().push_back_child<::resin::GroupNode>(::resin::SDFBinaryOperation::Union);
  } else if (op == SDFTreeOperationVisitor::Operation::PushPrimitive) {
    node.parent().push_back_child<::resin::SphereNode>(::resin::SDFBinaryOperation::Union);
  }
}

std::optional<::resin::IdView<::resin::SDFTreeNodeId>> SDFTreeView(::resin::SDFTree& tree) {
  ImGui::PushID(static_cast<int>(tree.tree_id()));
  static std::optional<::resin::IdView<::resin::SDFTreeNodeId>> selected = std::nullopt;

  auto comp_vs = resin::SDFTreeComponentVisitor(selected, tree.tree_id());
  resin::SDFTreeOperationVisitor op_vs;

  if (ImGui::BeginChild("ResizableChild", ImVec2(-FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * 8),
                        ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY)) {
    comp_vs.visit_root(tree.root());
    selected = comp_vs.selected();
  }
  ImGui::EndChild();

  comp_vs.apply_move_operation(tree);

  ImGui::IsItemClicked();
  ImGui::GetMouseDragDelta();

  bool delete_disabled = !selected.has_value();
  if (delete_disabled) {
    ImGui::BeginDisabled();
  }

  if (ImGui::Button("Delete")) {
    tree.delete_node(selected.value());
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

  if (ImGui::Button("Add Group")) {
    if (comp_vs.selected().has_value()) {
      op_vs.op = SDFTreeOperationVisitor::Operation::PushGroup;
      tree.node(comp_vs.selected().value()).accept_visitor(op_vs);
    }
  }

  ImGui::SameLine();

  const char* names[] = {"Sphere", "Cube"};
  if (ImGui::Button("Add")) {
    ImGui::OpenPopup("AddPopUp");
  }
  ImGui::SameLine();
  if (ImGui::BeginPopup("AddPopUp")) {
    ImGui::SeparatorText("Primitives");
    for (auto& name : names) {
      if (ImGui::Selectable(name)) {
        if (selected.has_value()) {
          op_vs.op = resin::SDFTreeOperationVisitor::Operation::PushPrimitive;
          tree.node(comp_vs.selected().value()).accept_visitor(op_vs);
        }
      }
    }
    ImGui::EndPopup();
  }

  if (push_disabled) {
    ImGui::EndDisabled();
  }
  ImGui::PopID();

  return selected;
}

}  // namespace resin

}  // namespace ImGui
