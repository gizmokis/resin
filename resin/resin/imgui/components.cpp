#include <imgui/imgui.h>

#include <cfloat>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <resin/imgui/components.hpp>

namespace ImGui {

bool resin::TransformEdit(::resin::Transform* transform) {
  bool value_changed = false;

  BeginGroup();
  if (ImGui::BeginTable("##Transform", 2)) {
    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Value");

    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("Position");

    ImGui::TableNextColumn();
    if (ImGui::DragFloat3("##Pos", glm::value_ptr(transform->local_pos()), 0.01F, 0.0F, 0.0F, "%.2f")) {
      value_changed = true;
      transform->mark_dirty();
    }
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("Rotation");

    ImGui::TableNextColumn();
    const glm::quat& rot = transform->local_rot();
    glm::vec3 euler      = glm::degrees(glm::eulerAngles(rot));

    if (ImGui::DragFloat3("##Rot", glm::value_ptr(euler), 0.1F, -180.F, 180.F, "%.1f°")) {
      value_changed = true;
      transform->set_local_rot(glm::quat(glm::radians(euler)));
    }

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text("Scale");
    ImGui::TableNextColumn();
    if (ImGui::DragFloat("##Scale", &transform->local_scale(), 0.01F, 0.F, FLT_MAX, "%.2f")) {
      value_changed = true;
      transform->mark_dirty();
    }

    ImGui::EndTable();
  }
  ImGui::EndGroup();
  return value_changed;
}

}  // namespace ImGui