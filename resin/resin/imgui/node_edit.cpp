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

namespace ImGui {
static void edit_mat_tab(::resin::Material& mat) {
  // TODO(SDF-87)
}

static void edit_op(::resin::SDFTreeNode& node) {
  auto curr_item = static_cast<int>(node.bin_op());

  if (ImGui::BeginCombo("Operation",
                        ::resin::sdf_shader_consts::kSDFShaderBinOpFunctionNames.get_value(node.bin_op()).data())) {
    for (const auto [index, name] :
         std::ranges::views::enumerate(::resin::sdf_shader_consts::kSDFShaderBinOpFunctionNames.names_)) {
      const bool is_selected = (index == curr_item);
      if (ImGui::Selectable(name.data(), is_selected)) {
        node.set_bin_op(static_cast<::resin::SDFBinaryOperation>(index));
      }
    }
    ImGui::EndCombo();
  }
}

void resin::SDFNodeEditVisitor::visit_sphere(::resin::SphereNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    ImGui::DragFloat("Radius", &node.radius, 0.01F, 0.0F, 1.0F, "%.2f");
    edit_op(node);
    ImGui::EndTabItem();
  }
  // edit_mat_tab(node.mat);
}

void resin::SDFNodeEditVisitor::visit_cube(::resin::CubeNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    ImGui::DragFloat("Size", &node.size, 0.01F, 0.0F, 1.0F, "%.2f");
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
      if (ImGui::resin::TransformEdit(&node.transform())) {
        node.mark_dirty();
      }
      ImGui::EndTabItem();
    }
    node.accept_visitor(vs);
    ImGui::EndTabBar();
  }
  return false;
}

}  // namespace resin

}  // namespace ImGui
