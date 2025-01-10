#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <filesystem>
#include <fstream>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <libresin/core/mesh_exporter.hpp>
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_base_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/json.hpp>
#include <libresin/utils/logger.hpp>
#include <memory>
#include <optional>
#include <ranges>
#include <resin/dialog/file_dialog.hpp>
#include <resin/imgui/sdf_tree.hpp>
#include <resin/resources/resource_managers.hpp>
#include <utility>

namespace ImGui {  // NOLINT

namespace resin {

static const std::array<::resin::FileDialog::FilterItem, 1> kPrefabFiltersArray = {
    ::resin::FileDialog::FilterItem("Resin prefab", "json")};

static const std::array<::resin::FileDialog::FilterItem, 2> kMeshFiltersArray = {
    ::resin::FileDialog::FilterItem("Wavefront obj", "obj"), ::resin::FileDialog::FilterItem("GLTF2", "gltf")};

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
  static const ImGuiTreeNodeFlags kBaseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                               ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding |
                                               ImGuiTreeNodeFlags_Selected;

  auto tree_flags = kBaseFlags;

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

  if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
    ImGui::OpenPopup("GroupPopUpMenu");
  }

  if (ImGui::BeginPopup("GroupPopUpMenu")) {
    if (ImGui::Selectable("Save as prefab")) {
      auto curr_id   = node.node_id();
      auto name      = node.name();
      auto& sdf_tree = sdf_tree_;

      ::resin::FileDialog::instance().save_file(
          [curr_id, &sdf_tree](const std::filesystem::path& path) {
            std::ofstream file(path);
            if (!file.is_open()) {
              ::resin::Logger::warn("Could not save to path {}", path.string());
              ::resin::log_throw(::resin::FileStreamNotAvailableException(path.string()));
              return;
            }

            file << ::resin::json::serialize_prefab(sdf_tree, curr_id);
            ::resin::Logger::info("Saved prefab to {}", path.string());
          },
          std::span<const ::resin::FileDialog::FilterItem>(kPrefabFiltersArray), std::string(name) += ".json");
    }
    if (node.primitives().size() > 0) {
      if (ImGui::BeginMenu("Export mesh as...")) {
        static int resolution_index      = 2;  // default to 32
        const unsigned int resolutions[] = {8, 16, 32, 64, 128, 256};
        const char* resolution_labels[]  = {"8", "16", "32", "64", "128", "256"};
        auto curr_id                     = node.node_id();
        auto name                        = node.name();
        auto& sdf_tree                   = sdf_tree_;
        ImGui::Text("Select resolution:");
        if (ImGui::Combo("##Resolution", &resolution_index, resolution_labels, IM_ARRAYSIZE(resolution_labels))) {
        }
        unsigned int resolution = resolutions[resolution_index];
        if (ImGui::MenuItem("OBJ")) {
          ::resin::FileDialog::instance().save_file(
              [curr_id, &sdf_tree, resolution](const std::filesystem::path& path) {
                ::resin::ShaderResourceManager resource_manager = ::resin::ResourceManagers::shader_manager();
                ::resin::ShaderResource shader_resource =
                    *resource_manager.get_res(std::filesystem::current_path() / "assets/marching_cubes.comp");
                ::resin::MeshExporter exporter(shader_resource, resolution);
                glm::vec3 pos = sdf_tree.group(curr_id).transform().pos();  // TODO(SDF-130) calculate bounding box
                exporter.setup_scene(pos - glm::vec3(5.0F), pos + glm::vec3(5.0F), sdf_tree, curr_id);
                exporter.export_mesh(path, "obj");
              },
              std::span<const ::resin::FileDialog::FilterItem>(kMeshFiltersArray), std::string(name) + ".obj");
        }

        if (ImGui::MenuItem("GLTF")) {
          ::resin::FileDialog::instance().save_file(
              [curr_id, &sdf_tree, resolution](const std::filesystem::path& path) {
                ::resin::ShaderResourceManager resource_manager = ::resin::ResourceManagers::shader_manager();
                ::resin::ShaderResource shader_resource =
                    *resource_manager.get_res(std::filesystem::current_path() / "assets/marching_cubes.comp");
                ::resin::MeshExporter exporter(shader_resource, resolution);
                glm::vec3 pos = sdf_tree.group(curr_id).transform().pos();  // TODO(SDF-130) calculate bounding box
                exporter.setup_scene(pos - glm::vec3(5.0F), pos + glm::vec3(5.0F), sdf_tree, curr_id);
                exporter.export_mesh(path, "gltf2");
              },
              std::span<const ::resin::FileDialog::FilterItem>(kMeshFiltersArray), std::string(name) + ".gltf");
        }
        ImGui::EndMenu();
      }
    }
    ImGui::EndPopup();
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
  if (is_first_ && node.bin_op() != ::resin::SDFBinaryOperation::SmoothUnion) {
    ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled), " (%s)",
                       kOperationSymbol.value(node.bin_op()).data());
    if (ImGui::BeginItemTooltip()) {
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 36.0F);
      ImGui::TextUnformatted("This operation is ignored for the first element in the group");
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
    }
  } else {
    ImGui::Text(" (%s)", kOperationSymbol.value(node.bin_op()).data());
  }
}

void SDFTreeComponentVisitor::render_tree() {
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0F, 0.0F});
  is_first_ = true;
  for (auto child_it = sdf_tree_.root().begin(); child_it != sdf_tree_.root().end(); ++child_it) {
    is_parent_selected_ = false;
    is_parent_dragged_  = false;
    sdf_tree_.root().get_child(*child_it).accept_visitor(*this);
    is_first_ = false;
  }
  ImGui::PopStyleVar();

  auto source_id = get_curr_payload();
  if (source_id.has_value()) {
    ImGui::BeginTooltipEx(ImGuiTooltipFlags_OverridePrevious, ImGuiWindowFlags_None);
    ImGui::Text("%s", sdf_tree_.node(*source_id).name().data());
    ImGui::EndTooltip();
  }
  auto child_rect = ImGui::GetCurrentWindow()->Rect();
  if (ImGui::IsMouseHoveringRect(child_rect.Min, child_rect.Max) && IsMouseClicked(ImGuiMouseButton_Left) &&
      !is_any_node_clicked_) {
    selected_ = std::nullopt;
  }
}

std::unique_ptr<::resin::SDFTreeNode> SDFTreeComponentVisitor::fix_transform_and_detach(
    ::resin::IdView<::resin::SDFTreeNodeId> source, ::resin::IdView<::resin::SDFTreeNodeId> new_parent) {
  if (sdf_tree_.node(source).parent().node_id() == new_parent) {
    auto node_ptr = sdf_tree_.node(source).parent().detach_child(source);
    return node_ptr;
  }

  // this way from the user point of view transform of the node will not change
  const auto& old_world_mat            = sdf_tree_.node(source).transform().local_to_world_matrix();
  const auto& new_parent_world_inv_mat = sdf_tree_.node(new_parent).transform().world_to_local_matrix();
  auto new_loc_mat                     = new_parent_world_inv_mat * old_world_mat;

  // decompose new mat
  auto new_loc_pos    = glm::vec3(new_loc_mat[3]);
  auto new_loc_rot    = glm::normalize(glm::quat_cast(new_loc_mat));
  float new_loc_scale = glm::length(glm::vec3(new_loc_mat[0][0], new_loc_mat[1][0], new_loc_mat[2][0]));

  auto node_ptr = sdf_tree_.node(source).parent().detach_child(source);
  node_ptr->transform().set_local_pos(new_loc_pos);
  node_ptr->transform().set_local_rot(new_loc_rot);
  node_ptr->transform().set_local_scale(new_loc_scale);

  return node_ptr;
}

void SDFTreeComponentVisitor::apply_move_operation() {
  if (!move_source_target_.has_value() || move_source_target_->expired()) {
    return;
  }

  if (move_into_target_.has_value()                                                     //
      && !move_into_target_->expired()                                                  //
      && sdf_tree_.node(*move_source_target_).parent().node_id() != *move_into_target_  //
      && sdf_tree_.node(*move_source_target_).node_id() != *move_into_target_           //
      && sdf_tree_.is_group(*move_into_target_)                                         //
  ) {
    auto node_ptr = fix_transform_and_detach(*move_source_target_, *move_into_target_);
    sdf_tree_.group(*move_into_target_).push_back_child(std::move(node_ptr));
    return;
  }

  if (move_before_target_.has_value() && !move_before_target_->expired()) {
    auto node_ptr =
        fix_transform_and_detach(*move_source_target_, sdf_tree_.node(*move_before_target_).parent().node_id());
    sdf_tree_.node(*move_before_target_).parent().insert_before_child(*move_before_target_, std::move(node_ptr));
    return;
  }

  if (move_after_target_.has_value() && !move_after_target_->expired()) {
    auto node_ptr =
        fix_transform_and_detach(*move_source_target_, sdf_tree_.node(*move_after_target_).parent().node_id());
    sdf_tree_.node(*move_after_target_).parent().insert_after_child(*move_after_target_, std::move(node_ptr));
    return;
  }
}

std::optional<::resin::IdView<::resin::SDFTreeNodeId>> SDFTreeView(
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

  auto comp_vs = resin::SDFTreeComponentVisitor(tree, old_selected);

  ImGui::BeginChild("ResizableInnerChild", ImVec2(-FLT_MIN, ImGui::GetWindowHeight() - buttons_section_height));

  comp_vs.render_tree();
  auto selected = comp_vs.selected();

  ImGui::EndChild();

  comp_vs.apply_move_operation();

  ImGui::IsItemClicked();
  ImGui::GetMouseDragDelta();

  bool delete_disabled = !selected.has_value();
  if (delete_disabled) {
    ImGui::BeginDisabled();
  }

  if (ImGui::Button(delete_label.data())) {
    tree.delete_node(selected.value());
    selected = std::nullopt;
  }

  if (delete_disabled) {
    ImGui::EndDisabled();
  }

  ImGui::SameLine(ImGui::GetWindowWidth() - add_buttons_section_width);
  if (ImGui::Button(add_group_label.data())) {
    ImGui::OpenPopup("AddGroupPopUp");
  }

  if (ImGui::BeginPopup("AddGroupPopUp")) {
    if (ImGui::Selectable("New")) {
      if (selected.has_value()) {
        if (tree.is_group(*selected)) {
          tree.group(*selected).push_back_child<::resin::GroupNode>(::resin::SDFBinaryOperation::SmoothUnion);
        } else {
          tree.node(*selected).parent().push_back_child<::resin::GroupNode>(::resin::SDFBinaryOperation::SmoothUnion);
        }
      } else {
        tree.root().push_back_child<::resin::GroupNode>(::resin::SDFBinaryOperation::SmoothUnion);
      }
    }

    if (ImGui::Selectable("Load from prefab")) {
      auto& sdf_tree = tree;

      ::resin::FileDialog::instance().open_file(
          [&sdf_tree, selected](const std::filesystem::path& path) {
            std::string json_content;
            std::ifstream file(path);
            if (!file.is_open()) {
              ::resin::Logger::warn("Could not open a file with path {}", path.string());
              ::resin::log_throw(::resin::FileStreamNotAvailableException(path.string()));
              return;
            }

            std::ostringstream ss;
            ss << file.rdbuf();
            json_content = ss.str();

            auto group = ::resin::json::deserialize_prefab(sdf_tree, json_content);
            if (selected.has_value()) {
              if (sdf_tree.is_group(*selected)) {
                sdf_tree.group(*selected).push_back_child(std::move(group));
              } else {
                sdf_tree.node(*selected).parent().push_back_child(std::move(group));
              }
            } else {
              sdf_tree.root().push_back_child(std::move(group));
            }
            ::resin::Logger::info("Loaded prefab from {}", path.string());
          },
          std::span<const ::resin::FileDialog::FilterItem>(kPrefabFiltersArray));
    }
    ImGui::EndPopup();
  }

  ImGui::SameLine(ImGui::GetWindowWidth() - add_prim_button_width);
  if (ImGui::Button(add_prim_label.data())) {
    ImGui::OpenPopup("AddPrimitivePopUp");
  }

  if (ImGui::BeginPopup("AddPrimitivePopUp")) {
    for (const auto [prim, name] : ::resin::BasePrimitiveNode::available_primitive_names()) {
      if (ImGui::Selectable(name.data())) {
        if (selected.has_value()) {
          if (tree.is_group(*selected)) {
            tree.group(*selected).push_back_primitive(prim, ::resin::SDFBinaryOperation::SmoothUnion);
          } else {
            tree.node(*selected).parent().push_back_primitive(prim, ::resin::SDFBinaryOperation::SmoothUnion);
          }
        } else {
          tree.root().push_back_primitive(prim, ::resin::SDFBinaryOperation::SmoothUnion);
        }
      }
    }
    ImGui::EndPopup();
  }

  ImGui::PopID();

  return selected;
}

}  // namespace resin

}  // namespace ImGui
