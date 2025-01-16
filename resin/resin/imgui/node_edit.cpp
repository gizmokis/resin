#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <libresin/core/material.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_base_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <ranges>
#include <resin/imgui/node_edit.hpp>
#include <resin/imgui/transform_edit.hpp>

#define NODE_DIRTY(x) \
  if (x) node.mark_dirty()

namespace ImGui {

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

static void edit_mat(::resin::SDFTreeNode& node) {}

void resin::SDFNodeEditVisitor::visit_sphere(::resin::SphereNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    NODE_DIRTY(ImGui::DragFloat("Radius", &node.radius, 0.01F, 0.0F, 2.0F, "%.2f"));
    edit_op(node);
    ImGui::EndTabItem();
  }
}

void resin::SDFNodeEditVisitor::visit_cube(::resin::CubeNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    NODE_DIRTY(ImGui::DragFloat3("Size", glm::value_ptr(node.size), 0.01F, 0.0F, 2.0F, "%.2f"));
    edit_op(node);
    ImGui::EndTabItem();
  }
  // edit_mat_tab(node.mat);
}

void resin::SDFNodeEditVisitor::visit_torus(::resin::TorusNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    NODE_DIRTY(ImGui::DragFloat("Major radius", &node.major_radius, 0.01F, 0.0F, 2.0F, "%.2f"));
    NODE_DIRTY(ImGui::DragFloat("Minor radius", &node.minor_radius, 0.01F, 0.0F, 2.0F, "%.2f"));
    edit_op(node);
    ImGui::EndTabItem();
  }
  // edit_mat_tab(node.mat);
}

void resin::SDFNodeEditVisitor::visit_capsule(::resin::CapsuleNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    NODE_DIRTY(ImGui::DragFloat("Height", &node.height, 0.01F, 0.0F, 2.0F, "%.2f"));
    NODE_DIRTY(ImGui::DragFloat("Radius", &node.radius, 0.01F, 0.0F, 2.0F, "%.2f"));
    edit_op(node);
    ImGui::EndTabItem();
  }
  // edit_mat_tab(node.mat);
}

void resin::SDFNodeEditVisitor::visit_link(::resin::LinkNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    NODE_DIRTY(ImGui::DragFloat("Length", &node.length, 0.01F, 0.0F, 2.0F, "%.2f"));
    NODE_DIRTY(ImGui::DragFloat("Major radius", &node.major_radius, 0.01F, 0.0F, 2.0F, "%.2f"));
    NODE_DIRTY(ImGui::DragFloat("Minor radius", &node.minor_radius, 0.01F, 0.0F, 2.0F, "%.2f"));
    edit_op(node);
    ImGui::EndTabItem();
  }
  // edit_mat_tab(node.mat);
}

void resin::SDFNodeEditVisitor::visit_ellipsoid(::resin::EllipsoidNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    NODE_DIRTY(ImGui::DragFloat3("Size", glm::value_ptr(node.radii), 0.01F, 0.0F, 2.0F, "%.2f"));
    edit_op(node);
    ImGui::EndTabItem();
  }
  // edit_mat_tab(node.mat);
}

void resin::SDFNodeEditVisitor::visit_pyramid(::resin::PyramidNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    NODE_DIRTY(ImGui::DragFloat("Height", &node.height, 0.01F, 0.0F, 2.0F, "%.2f"));
    edit_op(node);
    ImGui::EndTabItem();
  }
  // edit_mat_tab(node.mat);
}

void resin::SDFNodeEditVisitor::visit_cylinder(::resin::CylinderNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    NODE_DIRTY(ImGui::DragFloat("Height", &node.height, 0.01F, 0.0F, 2.0F, "%.2f"));
    NODE_DIRTY(ImGui::DragFloat("Radius", &node.radius, 0.01F, 0.0F, 2.0F, "%.2f"));
    edit_op(node);
    ImGui::EndTabItem();
  }
  // edit_mat_tab(node.mat);
}

void resin::SDFNodeEditVisitor::visit_prism(::resin::TriangularPrismNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    NODE_DIRTY(ImGui::DragFloat("Prism Height", &node.prismHeight, 0.01F, 0.0F, 2.0F, "%.2f"));
    NODE_DIRTY(ImGui::DragFloat("Base Height", &node.baseHeight, 0.01F, 0.0F, 2.0F, "%.2f"));
    edit_op(node);
    ImGui::EndTabItem();
  }
}

void resin::SDFNodeEditVisitor::visit_group(::resin::GroupNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    edit_op(node);
    ImGui::EndTabItem();
  }
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
