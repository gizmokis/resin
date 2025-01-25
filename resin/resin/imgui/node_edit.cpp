#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <libresin/core/framebuffer.hpp>
#include <libresin/core/material.hpp>
#include <libresin/core/raycaster.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_base_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/shader.hpp>
#include <ranges>
#include <resin/imgui/material.hpp>
#include <resin/imgui/node_edit.hpp>
#include <resin/imgui/transform_edit.hpp>

#define NODE_DIRTY(x) \
  if (x) node.mark_primitives_dirty()

namespace ImGui {

namespace resin {

void resin::SDFNodeEditVisitor::visit_sphere(::resin::SphereNode& node) {
  NODE_DIRTY(ImGui::DragFloat("Radius", &node.radius, 0.01F, 0.0F, 2.0F, "%.2f"));
}

void resin::SDFNodeEditVisitor::visit_cube(::resin::CubeNode& node) {
  NODE_DIRTY(ImGui::DragFloat3("Size", glm::value_ptr(node.size), 0.01F, 0.0F, 2.0F, "%.2f"));
}

void resin::SDFNodeEditVisitor::visit_torus(::resin::TorusNode& node) {
  NODE_DIRTY(ImGui::DragFloat("Major radius", &node.major_radius, 0.01F, 0.0F, 2.0F, "%.2f"));
  NODE_DIRTY(ImGui::DragFloat("Minor radius", &node.minor_radius, 0.01F, 0.0F, 2.0F, "%.2f"));
}

void resin::SDFNodeEditVisitor::visit_capsule(::resin::CapsuleNode& node) {
  NODE_DIRTY(ImGui::DragFloat("Height", &node.height, 0.01F, 0.0F, 2.0F, "%.2f"));
  NODE_DIRTY(ImGui::DragFloat("Radius", &node.radius, 0.01F, 0.0F, 2.0F, "%.2f"));
}

void resin::SDFNodeEditVisitor::visit_link(::resin::LinkNode& node) {
  NODE_DIRTY(ImGui::DragFloat("Length", &node.length, 0.01F, 0.0F, 2.0F, "%.2f"));
  NODE_DIRTY(ImGui::DragFloat("Major radius", &node.major_radius, 0.01F, 0.0F, 2.0F, "%.2f"));
  NODE_DIRTY(ImGui::DragFloat("Minor radius", &node.minor_radius, 0.01F, 0.0F, 2.0F, "%.2f"));
}

void resin::SDFNodeEditVisitor::visit_ellipsoid(::resin::EllipsoidNode& node) {
  NODE_DIRTY(ImGui::DragFloat3("Size", glm::value_ptr(node.radii), 0.01F, 0.0F, 2.0F, "%.2f"));
}

void resin::SDFNodeEditVisitor::visit_pyramid(::resin::PyramidNode& node) {
  NODE_DIRTY(ImGui::DragFloat("Height", &node.height, 0.01F, 0.0F, 2.0F, "%.2f"));
}

void resin::SDFNodeEditVisitor::visit_cylinder(::resin::CylinderNode& node) {
  NODE_DIRTY(ImGui::DragFloat("Height", &node.height, 0.01F, 0.0F, 2.0F, "%.2f"));
  NODE_DIRTY(ImGui::DragFloat("Radius", &node.radius, 0.01F, 0.0F, 2.0F, "%.2f"));
}

void resin::SDFNodeEditVisitor::visit_prism(::resin::TriangularPrismNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    NODE_DIRTY(ImGui::DragFloat("Prism Height", &node.prismHeight, 0.01F, 0.0F, 2.0F, "%.2f"));
    NODE_DIRTY(ImGui::DragFloat("Base Height", &node.baseHeight, 0.01F, 0.0F, 2.0F, "%.2f"));
    ImGui::EndTabItem();
  }
}

bool NodeEdit(::resin::SDFTreeNode& node, LazyMaterialImageFramebuffers& material_img_fbs,
              std::optional<::resin::IdView<::resin::MaterialId>>& selected_material,
              const ::resin::SDFTree& sdf_tree) {
  static std::optional<::resin::IdView<::resin::MaterialId>> modal_selected_material;

  SDFNodeEditVisitor vs;

  if (ImGui::BeginTabBar("NodeTabBar", ImGuiTabBarFlags_None)) {
    if (ImGui::BeginTabItem("Transform")) {
      if (ImGui::resin::TransformEdit(&node.transform())) {
        node.mark_dirty();
        node.mark_primitives_dirty();
      }
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Properties")) {
      if (ImGui::BeginCombo("Operation",
                            ::resin::sdf_shader_consts::kSDFShaderBinOpFunctionNames[node.bin_op()].data())) {
        for (const auto [current_op, name] : ::resin::sdf_shader_consts::kSDFShaderBinOpFunctionNames) {
          const bool is_selected = (current_op == node.bin_op());
          if (ImGui::Selectable(name.data(), is_selected)) {
            node.set_bin_op(current_op);
          }
        }
        ImGui::EndCombo();
      }

      if (node.has_smooth_bin_op()) {
        float factor = node.factor();
        if (ImGui::DragFloat("Factor", &factor, 0.01F, 0.F, 2.F, "%.2F")) {
          node.set_factor(factor);
        }
      }

      node.accept_visitor(vs);

      const auto img_size = static_cast<float>(material_img_fbs.node_material_preview_fb.fb.width());
      if (node.material_id() && !node.material_id()->expired()) {
        if (node.material_id() != material_img_fbs.node_material_preview_id) {
          material_img_fbs.node_material_preview_id = node.material_id();
          material_img_fbs.node_material_preview_fb.mark_dirty();
        }

        ImGui::ImageButton(
            "##MaterialButton",                                                                   //
            (ImTextureID)(intptr_t)material_img_fbs.node_material_preview_fb.fb.color_texture(),  // NOLINT
            ImVec2(img_size, img_size),                                                           //
            ImVec2(0, 1),                                                                         //
            ImVec2(1, 0)                                                                          //
        );

      } else {
        ImGui::Button("##MaterialButton", ImVec2(img_size, img_size));
      }

      if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        if (!node.material_id()) {
          ImGui::OpenPopup("Select material");
          modal_selected_material = node.material_id();
        } else {
          selected_material = node.material_id();
        }
      }

      if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        ImGui::OpenPopup("Select material");
        modal_selected_material = node.material_id();
      }

      if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL_PAYLOAD")) {
          IM_ASSERT(payload->DataSize == sizeof(::resin::IdView<::resin::MaterialId>));
          auto source_id = *static_cast<const ::resin::IdView<::resin::MaterialId>*>(payload->Data);

          if (!source_id.expired()) {
            node.set_material(source_id);
            material_img_fbs.node_material_preview_id = source_id;
            material_img_fbs.node_material_preview_fb.mark_dirty();
          }
        }
      }

      if (node.material_id()) {
        ImGui::SameLine();
        ImGui::Text("%s", sdf_tree.material(*node.material_id()).name().data());
        if (ImGui::Button("Remove")) {
          node.remove_material();
        }
      }

      if (ImGui::BeginPopupModal("Select material", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!modal_selected_material) {
          ImGui::BeginDisabled();
        }

        if (ImGui::Button("OK", ImVec2(120, 0))) {
          if (modal_selected_material && !modal_selected_material->expired()) {
            selected_material = modal_selected_material;
            node.set_material(*modal_selected_material);
          }
          ImGui::CloseCurrentPopup();
        }

        if (!modal_selected_material) {
          ImGui::EndDisabled();
        }

        ImGui::SetItemDefaultFocus();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
          ImGui::CloseCurrentPopup();
        }

        ImGui::SetNextWindowSizeConstraints(ImVec2(240.F, 200.F), ImVec2(FLT_MAX, FLT_MAX));
        MaterialsList(modal_selected_material, material_img_fbs, sdf_tree);

        ImGui::EndPopup();
      }

      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }

  return false;
}

}  // namespace resin

}  // namespace ImGui
