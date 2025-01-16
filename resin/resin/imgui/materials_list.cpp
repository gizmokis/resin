#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <libresin/core/framebuffer.hpp>
#include <libresin/core/id_registry.hpp>
#include <libresin/core/material.hpp>
#include <libresin/core/raycaster.hpp>
#include <memory>
#include <resin/imgui/materials_list.hpp>

namespace ImGui {

namespace resin {

static void render_img(::resin::ImageFramebuffer& framebuffer,
                       const ::resin::RenderingShaderProgram& material_image_shader) {
  static ::resin::Raycaster raycaster;
  framebuffer.bind();
  framebuffer.clear();
  raycaster.bind();

  material_image_shader.bind();
  raycaster.draw_call();
  material_image_shader.unbind();

  framebuffer.unbind();
}

std::optional<::resin::IdView<::resin::MaterialId>> MaterialsList(
    MaterialFramebuffers& material_view_framebuffers, const ::resin::RenderingShaderProgram& material_image_shader,
    const ::resin::SDFTree& sdf_tree, const std::optional<::resin::IdView<::resin::MaterialId>>& selected_old,
    size_t img_size) {
  static const float padding = 4.0F;
  auto selected              = selected_old;

  auto cols = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / (2 * padding + img_size)));
  ImGui::BeginTable("MaterialsTable", cols);

  for (const auto& m : sdf_tree.materials()) {
    ImGui::PushID(static_cast<int>(m.raw()));
    auto it = material_view_framebuffers.find(m);
    if (it == material_view_framebuffers.end()) {
      it = material_view_framebuffers.emplace(m, std::make_unique<::resin::ImageFramebuffer>(img_size, img_size)).first;
    }

    bool is_selected = selected_old && *selected_old == m;
    auto& mat        = sdf_tree.material(m);

    // TEMP
    material_image_shader.set_uniform("u_material", mat.material);
    render_img(*it->second, material_image_shader);

    if (!is_selected) {
      ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
    }
    if (ImGui::ImageButton("Material", (ImTextureID)(intptr_t)it->second->color_texture(), ImVec2(img_size, img_size),
                           ImVec2(0, 1),  // NOLINT
                           ImVec2(1, 0))) {
      selected = std::make_optional(m);
    }
    ImGui::TextWrapped("%s", mat.name().data());
    if (!is_selected) {
      ImGui::PopStyleColor();
    }

    ImGui::TableNextColumn();

    ImGui::PopID();
  }
  ImGui::EndTable();

  return selected;
}

}  // namespace resin

}  // namespace ImGui
