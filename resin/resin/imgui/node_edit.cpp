#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <libresin/core/material.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/primitive_base_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <ranges>
#include <resin/imgui/node_edit.hpp>
#include <resin/imgui/transform_edit.hpp>

#define NODE_DIRTY(x) \
  if (x) node.mark_dirty()

namespace ImGui {
static void edit_mat_tab(::resin::Material& mat) {
  // TODO(SDF-87)
  if (ImGui::BeginTabItem("Material")) {
    ImGui::ColorEdit3("Color", glm::value_ptr(mat.albedo));
    ImGui::DragFloat("Ambient", &mat.ambientFactor, 0.01F, 0.0F, 1.0F, "%.2f");
    ImGui::DragFloat("Diffuse", &mat.diffuseFactor, 0.01F, 0.0F, 1.0F, "%.2f");
    ImGui::DragFloat("Specular", &mat.specularFactor, 0.01F, 0.0F, 1.0F, "%.2f");
    ImGui::DragFloat("Exponent", &mat.specularExponent, 0.1F, 0.0F, 100.0F, "%.1f");
    ImGui::EndTabItem();
  }
}

static void edit_op(::resin::SDFTreeNode& node) {
  if (ImGui::BeginCombo("Operation", ::resin::sdf_shader_consts::kSDFShaderBinOpFunctionNames[node.bin_op()].data())) {
    for (const auto [current_op, name] : ::resin::sdf_shader_consts::kSDFShaderBinOpFunctionNames) {
      const bool is_selected = (current_op == node.bin_op());
      if (ImGui::Selectable(name.data(), is_selected)) {
        node.set_bin_op(current_op);
      }
    }
    ImGui::EndCombo();
  }
}

void resin::SDFNodeEditVisitor::visit_sphere(::resin::SphereNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    NODE_DIRTY(ImGui::DragFloat("Radius", &node.radius, 0.01F, 0.0F, 1.0F, "%.2f"));
    edit_op(node);
    ImGui::EndTabItem();
  }
  // edit_mat_tab(node.mat);
}

void resin::SDFNodeEditVisitor::visit_cube(::resin::CubeNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    NODE_DIRTY(ImGui::DragFloat("Size", &node.size, 0.01F, 0.0F, 1.0F, "%.2f"));
    edit_op(node);
    ImGui::EndTabItem();
  }
  // edit_mat_tab(node.mat);
}

namespace resin {

bool NodeEdit(::resin::SDFTreeNode& node) {
  static SDFNodeEditVisitor vs;

  if (ImGui::BeginTabBar("NodeTabBar", ImGuiTabBarFlags_None)) {
    if (ImGui::BeginTabItem("Transform")) {
      NODE_DIRTY(ImGui::resin::TransformEdit(&node.transform()));
      ImGui::EndTabItem();
    }
    node.accept_visitor(vs);
    ImGui::EndTabBar();
  }
  return false;
}

}  // namespace resin

}  // namespace ImGui
