#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <libresin/core/sdf_tree/primitive_base_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <resin/imgui/node_edit.hpp>
#include <resin/imgui/transform_edit.hpp>

namespace ImGui {

void resin::SDFNodeEditVisitor::visit_sphere(::resin::SphereNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    ImGui::DragFloat("Radius", &node.radius, 0.01F, 0.0F, 1.0F, "%.2f");
    ImGui::EndTabItem();
  }
}

void resin::SDFNodeEditVisitor::visit_cube(::resin::CubeNode& node) {
  if (ImGui::BeginTabItem("Properties")) {
    ImGui::DragFloat("Size", &node.size, 0.01F, 0.0F, 1.0F, "%.2f");
    ImGui::EndTabItem();
  }
}

void resin::SDFNodeEditVisitor::visit_primitive(::resin::BasePrimitiveNode& node) {
  // TODO(SDF-87)
}

namespace resin {

bool NodeEdit(::resin::SDFTreeNode& node) {
  static SDFNodeEditVisitor vs;

  if (ImGui::BeginTabBar("NodeTabBar", ImGuiTabBarFlags_None)) {
    if (ImGui::BeginTabItem("Transform")) {
      if(ImGui::resin::TransformEdit(&node.transform())) {
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
