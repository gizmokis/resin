#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <libresin/core/framebuffer.hpp>
#include <libresin/core/id_registry.hpp>
#include <libresin/core/material.hpp>
#include <libresin/core/raycaster.hpp>
#include <libresin/utils/logger.hpp>
#include <memory>
#include <resin/imgui/dnd_payloads.hpp>
#include <resin/imgui/material.hpp>
#include <resin/imgui/types.hpp>

namespace ImGui {

namespace resin {

bool MaterialsList(std::optional<::resin::IdView<::resin::MaterialId>>& selected,
                   LazyMaterialImageFramebuffers& material_img_fbs, const ::resin::SDFTree& sdf_tree) {
  static const float kPadding = 6.0F;

  if (sdf_tree.materials().empty()) {
    return false;
  }

  auto selected_new       = selected;
  bool is_any_mat_clicked = false;
  const auto img_size     = static_cast<float>(material_img_fbs.material_preview_img_size);
  auto cols               = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / (2 * kPadding + img_size)));

  ImGui::BeginChild("ResizeableMaterialsList");
  if (ImGui::BeginTable("MaterialsTable", cols)) {
    ImGui::TableNextColumn();
    for (const auto& mat_id : sdf_tree.materials()) {
      ImGui::PushID(static_cast<int>(mat_id.raw()));

      // Add missing material framebuffers to the preview map
      auto it = material_img_fbs.material_preview_fbs_map.find(mat_id);
      if (it == material_img_fbs.material_preview_fbs_map.end()) {
        it = material_img_fbs               //
                 .material_preview_fbs_map  //
                 .emplace(mat_id,
                          std::make_unique<LazyImageFramebuffer>(::resin::ImageFramebuffer(
                              material_img_fbs.material_preview_img_size, material_img_fbs.material_preview_img_size)))
                 .first;

        it->second->mark_dirty();

        ::resin::Logger::info("Added material image framebuffer for material with id {}", it->first.raw());
      }

      bool is_selected = selected_new && *selected_new == mat_id;
      const auto& mat  = sdf_tree.material(mat_id);

      if (!is_selected) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
      };
      ImGui::ImageButton("##MaterialButton",                                     //
                         (ImTextureID)(intptr_t)it->second->fb.color_texture(),  // NOLINT
                         ImVec2(img_size, img_size),                             //
                         ImVec2(0, 1),                                           //
                         ImVec2(1, 0)                                            //
      );

      if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        selected_new       = std::make_optional(mat_id);
        is_any_mat_clicked = true;
      }

      if (!is_selected) {
        ImGui::PopStyleColor();
      }

      if (ImGui::BeginDragDropSource()) {
        auto curr_id = mat.material_id();
        ImGui::SetDragDropPayload(kMaterialPayloadName.data(), &curr_id, sizeof(::resin::IdView<::resin::MaterialId>));
        ImGui::Text("%s", mat.name().data());
        ImGui::EndDragDropSource();
      }

      ImGui::TextWrapped("%s", mat.name().data());

      ImGui::PopID();
      ImGui::TableNextColumn();
    }

    auto child_rect = ImGui::GetCurrentWindow()->Rect();
    if (!is_any_mat_clicked && ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
        ImGui::IsMouseHoveringRect(child_rect.Min, child_rect.Max)) {
      selected_new = std::nullopt;
    }

    ImGui::EndTable();
  }
  ImGui::EndChild();

  // Remove outdated material framebuffers if the number of stored framebuffers exceeds twice the number of materials.
  // This heuristic minimizes the number of framebuffer deletions and enables reusage of the already created
  // framebuffers.
  if (material_img_fbs.material_preview_fbs_map.size() >= 2 * sdf_tree.materials().size()) {
    std::erase_if(material_img_fbs.material_preview_fbs_map, [](const auto& pair) { return pair.first.expired(); });
    ::resin::Logger::info("Deleted obselete material framebuffers");
  }

  bool edited = selected != selected_new;
  selected    = selected_new;
  return edited;
}

bool MaterialsListEdit(std::optional<::resin::IdView<::resin::MaterialId>>& selected,
                       LazyMaterialImageFramebuffers& material_img_fbs, ::resin::SDFTree& sdf_tree) {
  auto selected_new = selected;
  bool edited       = false;

  if (selected_new && selected_new->expired()) {
    selected_new = std::nullopt;
  }

  if (ImGui::Button("Add")) {
    selected_new = sdf_tree.add_material(::resin::Material()).material_id();
    edited       = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("Delete")) {
    if (selected_new) {
      sdf_tree.delete_material(*selected_new);
    }
    edited = true;
  }

  edited = MaterialsList(selected_new, material_img_fbs, sdf_tree) || edited;

  if (selected_new && selected != selected_new) {
    material_img_fbs.material_preview_fbs_map[*selected_new]->mark_dirty();
  }

  selected = selected_new;

  return edited;
}

bool MaterialEdit(std::optional<::resin::IdView<::resin::MaterialId>>& selected,
                  LazyMaterialImageFramebuffers& material_img_fbs, ::resin::SDFTree& sdf_tree) {
  if (!selected || selected->expired()) {
    return false;
  }

  if (selected != material_img_fbs.main_material_id) {
    material_img_fbs.main_material_fb.mark_dirty();
    material_img_fbs.main_material_id = *selected;
  }

  const auto img_size = static_cast<float>(material_img_fbs.main_material_fb.fb.width());

  ImGui::Image((ImTextureID)(intptr_t)material_img_fbs.main_material_fb.fb.color_texture(),  // NOLINT
               ImVec2(img_size, img_size),                                                   //
               ImVec2(0, 1),                                                                 //
               ImVec2(1, 0)                                                                  //
  );

  auto& mat = sdf_tree.material(*selected);

  if (ImGui::Button("Delete")) {
    sdf_tree.delete_material(*selected);
    return true;
  }
  ImGui::SameLine();
  ImGui::Text("%s", mat.name().data());

  bool edited = false;
  edited      = ImGui::ColorEdit3("Color", glm::value_ptr(mat.material.albedo)) || edited;
  edited      = ImGui::DragFloat("Ambient", &mat.material.ambientFactor, 0.01F, 0.0F, 1.0F, "%.2f") || edited;
  edited      = ImGui::DragFloat("Diffuse", &mat.material.diffuseFactor, 0.01F, 0.0F, 1.0F, "%.2f") || edited;
  edited      = ImGui::DragFloat("Specular", &mat.material.specularFactor, 0.01F, 0.0F, 1.0F, "%.2f") || edited;
  edited      = ImGui::DragFloat("Exponent", &mat.material.specularExponent, 0.01F, 0.0F, 100.0F, "%.2f") || edited;

  if (edited) {
    auto it = material_img_fbs.material_preview_fbs_map.find(*selected);
    if (it != material_img_fbs.material_preview_fbs_map.end()) {
      it->second->mark_dirty();
    }

    material_img_fbs.main_material_fb.mark_dirty();

    if (material_img_fbs.node_material_preview_id == selected) {
      material_img_fbs.node_material_preview_fb.mark_dirty();
    }

    mat.mark_dirty();
  }

  return edited;
}

}  // namespace resin

}  // namespace ImGui
