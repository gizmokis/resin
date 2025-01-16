#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <libresin/core/framebuffer.hpp>
#include <libresin/core/id_registry.hpp>
#include <libresin/core/material.hpp>
#include <libresin/core/raycaster.hpp>
#include <memory>
#include <resin/imgui/material_edit.hpp>

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
    ::resin::SDFTree& sdf_tree, const std::optional<::resin::IdView<::resin::MaterialId>>& selected_old,
    size_t img_size) {
  static const float kPadding = 4.0F;
  auto selected               = selected_old;
  bool is_any_mat_clicked     = false;

  auto cols = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / (2 * kPadding + img_size)));
  ImGui::BeginTable("MaterialsTable", cols);

  for (const auto& m : sdf_tree.materials()) {
    ImGui::PushID(static_cast<int>(m.raw()));
    auto it = material_view_framebuffers.find(m);
    if (it == material_view_framebuffers.end()) {
      it = material_view_framebuffers.emplace(m, std::make_unique<::resin::ImageFramebuffer>(img_size, img_size)).first;
    }

    bool is_selected = selected_old && *selected_old == m;
    const auto& mat  = sdf_tree.material(m);

    if (mat.is_dirty()) {
      material_image_shader.set_uniform("u_material", mat.material);
      render_img(*it->second, material_image_shader);
    }

    if (!is_selected) {
      ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
    }
    if (ImGui::ImageButton("Material", (ImTextureID)(intptr_t)it->second->color_texture(), ImVec2(img_size, img_size),
                           ImVec2(0, 1),  // NOLINT
                           ImVec2(1, 0))) {
      selected           = std::make_optional(m);
      is_any_mat_clicked = true;
    }
    ImGui::TextWrapped("%s", mat.name().data());
    if (!is_selected) {
      ImGui::PopStyleColor();
    }

    ImGui::TableNextColumn();

    ImGui::PopID();
  }
  ImGui::EndTable();

  auto child_rect = ImGui::GetCurrentWindow()->Rect();
  if (!is_any_mat_clicked && ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
      ImGui::IsMouseHoveringRect(child_rect.Min, child_rect.Max)) {
    selected = std::nullopt;
  }

  if (selected && selected_old != selected) {
    sdf_tree.material(*selected).mark_dirty();
  }

  return selected;
}

bool MaterialEdit(::resin::ImageFramebuffer& framebuffer, const ::resin::RenderingShaderProgram& material_image_shader,
                  ::resin::MaterialSDFTreeComponent& mat, size_t img_size) {
  if (mat.is_dirty()) {
    material_image_shader.set_uniform("u_material", mat.material);
    render_img(framebuffer, material_image_shader);
  }
  ImGui::Image((ImTextureID)(intptr_t)framebuffer.color_texture(), ImVec2(img_size, img_size), ImVec2(0, 1),  // NOLINT
               ImVec2(1, 0));

  bool edited = false;
  edited      = ImGui::ColorEdit3("Color", glm::value_ptr(mat.material.albedo)) || edited;
  edited      = ImGui::DragFloat("Ambient", &mat.material.ambientFactor, 0.01F, 0.0F, 1.0F, "%.2f") || edited;
  edited      = ImGui::DragFloat("Diffuse", &mat.material.diffuseFactor, 0.01F, 0.0F, 1.0F, "%.2f") || edited;
  edited      = ImGui::DragFloat("Specular", &mat.material.specularFactor, 0.01F, 0.0F, 1.0F, "%.2f") || edited;
  edited      = ImGui::DragFloat("Exponent", &mat.material.specularExponent, 0.01F, 0.0F, 100.0F, "%.2f") || edited;

  if (edited) {
    mat.mark_dirty();
  }

  return edited;
}

}  // namespace resin

}  // namespace ImGui
