#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_base_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/utils/logger.hpp>
#include <memory>
#include <optional>
#include <ranges>
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
  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
    ImGui::SetDragDropPayload(payload_type_.c_str(), &curr_id, sizeof(::resin::IdView<::resin::SDFTreeNodeId>));
    ImGui::Text("%s", node.name().data());
    ImGui::EndDragDropSource();
  }
  auto dnd_col = ImGui::GetColorU32(ImGuiCol_DragDropTarget);

  if (ImGui::BeginDragDropTarget()) {
    ImVec2 item_min = ImGui::GetItemRectMin();
    ImVec2 item_max = ImGui::GetItemRectMax();

    float bottom_dist = item_max.y - ImGui::GetMousePos().y;
    bool middle       = !ignore_middle && ImGui::GetItemRectSize().y / 3.F * 2.F > bottom_dist &&
                  ImGui::GetItemRectSize().y / 3.F < bottom_dist;
    bool below = ImGui::GetItemRectSize().y / 2.F > bottom_dist;

    if (middle) {
      ImGui::GetForegroundDrawList()->AddRect(item_min, item_max, dnd_col);
    } else if (below) {
      ImGui::GetForegroundDrawList()->AddLine(ImVec2(item_min.x, item_max.y), item_max, dnd_col);
    } else {
      ImGui::GetForegroundDrawList()->AddLine(item_min, ImVec2(item_max.x, item_min.y), dnd_col);
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
  static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                         ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding |
                                         ImGuiTreeNodeFlags_Selected;

  auto tree_flags = base_flags;

  bool is_node_dragged = is_parent_dragged_;
  if (!is_node_dragged) {
    auto source_id  = get_curr_payload();
    is_node_dragged = source_id.has_value() && *source_id == node.node_id();
  }

  bool is_node_selected = is_parent_selected_ || selected_ == node.node_id();

  ImGui::PushID(static_cast<int>(node.node_id().raw()));

  if (is_node_dragged) {
    ImGui::BeginDisabled();
  }

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.F, 4.F));

  if (!is_node_selected) {
    ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
  }
  bool tree_node_opened = ImGui::TreeNodeEx(node.name().data(), tree_flags);
  if (!is_node_selected) {
    ImGui::PopStyleColor();
  }

  ImGui::PopStyleVar();

  if (is_node_dragged) {
    ImGui::EndDisabled();
  }

  if (ImGui::IsItemClicked()) {
    is_node_selected     = true;
    selected_            = node.node_id();
    is_any_node_clicked_ = true;
  }

  if (!is_node_dragged) {
    drag_and_drop(node, false);
  }

  render_op(node);

  if (!tree_node_opened) {
    ImGui::PopID();
    return;
  }

  is_first_ = true;
  for (auto child_it = node.begin(); child_it != node.end(); ++child_it) {
    is_parent_selected_ = is_node_selected;
    is_parent_dragged_  = is_node_dragged;
    node.get_child(*child_it).accept_visitor(*this);
    is_first_ = false;
  }
  is_first_ = false;

  ImGui::TreePop();
  ImGui::PopID();
}

void SDFTreeComponentVisitor::visit_primitive(::resin::BasePrimitiveNode& node) {
  auto source_id = get_curr_payload();

  bool is_node_selected = is_parent_selected_ || selected_ == node.node_id();
  bool is_node_dragged  = is_parent_dragged_ || (source_id.has_value() && *source_id == node.node_id());

  ImGui::PushID(static_cast<int>(node.node_id().raw()));
  if (is_node_dragged) {
    ImGui::BeginDisabled();
  }

  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen |
                             ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

  if (is_node_selected) {
    flags |= ImGuiTreeNodeFlags_Selected;
  }

  // TODO(SDF-100): Use primitive icons
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.F, 2.F));
  ImGui::TreeNodeEx(node.name().data(), flags);
  ImGui::PopStyleVar();

  if (ImGui::IsItemClicked()) {
    selected_            = node.node_id();
    is_any_node_clicked_ = true;
  }

  if (is_node_dragged) {
    ImGui::EndDisabled();
  }

  if (!is_node_dragged) {
    drag_and_drop(node, true);
  }

  render_op(node);

  ImGui::PopID();
}

void SDFTreeComponentVisitor::render_op(::resin::SDFTreeNode& node) const {
  ImGuiWindow* window = ImGui::GetCurrentWindow();
  ImGuiStyle& style   = ImGui::GetStyle();
  float op_offset     = ImGui::GetWindowWidth() - (window->ScrollbarY ? style.ScrollbarSize + 42.F : 42.F);

  // TODO(SDF-100): Use operation icons
  ImGui::SameLine(op_offset);
  if (is_first_ && node.bin_op() != ::resin::SDFBinaryOperation::Union) {
    ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled), " (%s)",
                       kOperationSymbol.get_value(node.bin_op()).data());
    if (ImGui::BeginItemTooltip()) {
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 36.0F);
      ImGui::TextUnformatted("This operation is ignored for the first element in the group");
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
    }
  } else {
    ImGui::Text(" (%s)", kOperationSymbol.get_value(node.bin_op()).data());
  }
}

void SDFTreeComponentVisitor::render_tree(::resin::SDFTree& tree) {
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0F, 0.0F});
  is_first_ = true;
  for (auto child_it = tree.root().begin(); child_it != tree.root().end(); ++child_it) {
    is_parent_selected_ = false;
    is_parent_dragged_  = false;
    tree.root().get_child(*child_it).accept_visitor(*this);
    is_first_ = false;
  }
  ImGui::PopStyleVar();

  auto source_id = get_curr_payload();
  if (source_id.has_value()) {
    ImGui::BeginTooltipEx(ImGuiTooltipFlags_OverridePrevious, ImGuiWindowFlags_None);
    ImGui::Text("%s", tree.node(*source_id).name().data());
    ImGui::EndTooltip();
  }
  auto child_rect = ImGui::GetCurrentWindow()->Rect();
  if (ImGui::IsMouseHoveringRect(child_rect.Min, child_rect.Max) && IsMouseClicked(ImGuiMouseButton_Left) &&
      !is_any_node_clicked_) {
    selected_ = std::nullopt;
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

  is_tree_edited_ = true;
}

std::pair<std::optional<::resin::IdView<::resin::SDFTreeNodeId>>, bool> SDFTreeView(
    ::resin::SDFTree& tree, const std::optional<::resin::IdView<::resin::SDFTreeNodeId>>& old_selected) {
  static std::string_view delete_label    = "Delete";
  static std::string_view add_prim_label  = "Add Primitive";
  static std::string_view add_group_label = "Add Group";

  ImGuiStyle& style            = ImGui::GetStyle();
  float buttons_section_height = ImGui::CalcTextSize(delete_label.data()).y + style.FramePadding.y * 4.0F +
                                 style.WindowPadding.y * 4.0F + style.ItemSpacing.y * 2.0F;
  float add_buttons_section_width = ImGui::CalcTextSize(add_prim_label.data()).x +
                                    ImGui::CalcTextSize(add_group_label.data()).x + style.FramePadding.x * 4.0F +
                                    style.ItemSpacing.y * 4.0F;
  float add_prim_button_width =
      ImGui::CalcTextSize(add_prim_label.data()).x + style.FramePadding.x * 2.0F + style.ItemSpacing.y * 2.0F;

  ImGui::PushID(static_cast<int>(tree.tree_id()));

  auto comp_vs = resin::SDFTreeComponentVisitor(old_selected, tree.tree_id());

  ImGui::BeginChild("ResizableInnerChild", ImVec2(-FLT_MIN, ImGui::GetWindowHeight() - buttons_section_height));

  comp_vs.render_tree(tree);
  auto selected = comp_vs.selected();
  comp_vs.apply_move_operation(tree);
  bool is_tree_edited = comp_vs.is_tree_edited();

  ImGui::EndChild();

  ImGui::IsItemClicked();
  ImGui::GetMouseDragDelta();

  bool delete_disabled = !selected.has_value();
  if (delete_disabled) {
    ImGui::BeginDisabled();
  }

  if (ImGui::Button(delete_label.data())) {
    tree.delete_node(selected.value());
    selected       = std::nullopt;
    is_tree_edited = true;
  }

  if (delete_disabled) {
    ImGui::EndDisabled();
  }

  ImGui::SameLine(ImGui::GetWindowWidth() - add_buttons_section_width);
  if (ImGui::Button(add_group_label.data())) {
    if (comp_vs.selected().has_value()) {
      if (tree.is_group(*selected)) {
        tree.group(*selected).push_back_child<::resin::GroupNode>(::resin::SDFBinaryOperation::SmoothUnion);
      } else {
        tree.node(*selected).parent().push_back_child<::resin::GroupNode>(::resin::SDFBinaryOperation::SmoothUnion);
      }
    } else {
      tree.root().push_back_child<::resin::GroupNode>(::resin::SDFBinaryOperation::SmoothUnion);
    }
    is_tree_edited = true;
  }

  ImGui::SameLine(ImGui::GetWindowWidth() - add_prim_button_width);
  if (ImGui::Button(add_prim_label.data())) {
    ImGui::OpenPopup("AddPopUp");
  }

  if (ImGui::BeginPopup("AddPopUp")) {
    for (const auto [index, name] :
         std::ranges::views::enumerate(::resin::BasePrimitiveNode::available_primitive_names())) {
      if (ImGui::Selectable(name.data())) {
        if (selected.has_value()) {
          if (tree.is_group(*selected)) {
            tree.group(*selected).push_back_primitive(static_cast<::resin::SDFTreePrimitiveType>(index),
                                                      ::resin::SDFBinaryOperation::SmoothUnion);
          } else {
            tree.node(*selected).parent().push_back_primitive(static_cast<::resin::SDFTreePrimitiveType>(index),
                                                              ::resin::SDFBinaryOperation::SmoothUnion);
          }
        } else {
          tree.root().push_back_primitive(static_cast<::resin::SDFTreePrimitiveType>(index),
                                          ::resin::SDFBinaryOperation::SmoothUnion);
        }
        is_tree_edited = true;
      }
    }
    ImGui::EndPopup();
  }

  ImGui::PopID();

  return std::make_pair(selected, is_tree_edited);
}

}  // namespace resin

}  // namespace ImGui
