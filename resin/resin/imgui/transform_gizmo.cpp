#include <imgui/imgui.h>
#include <imguizmo/ImGuizmo.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <resin/imgui/transform_gizmo.hpp>

namespace ImGui {  // NOLINT

namespace resin {

bool TranslationGizmo(::resin::Transform& trans, const ::resin::Camera& camera, GizmoMode mode, float width,
                      float height) {
  auto view      = camera.view_matrix();
  auto proj      = camera.proj_matrix();
  auto delta_mat = glm::mat4(1.0F);
  auto mat       = trans.local_to_world_matrix();

  ImGuizmo::BeginFrame();
  ImGuizmo::SetDrawlist();
  ImGuizmo::SetOrthographic(camera.is_orthographic);
  ImGuizmo::SetRect(ImGui::GetWindowPos().x + ImGui::GetCursorStartPos().x,
                    ImGui::GetWindowPos().y + ImGui::GetCursorStartPos().y, width, height);

  if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), ImGuizmo::OPERATION::TRANSLATE,
                           mode == GizmoMode::Local ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD,
                           glm::value_ptr(mat), glm::value_ptr(delta_mat))) {
    auto dp = glm::vec3(delta_mat[3][0], delta_mat[3][1], delta_mat[3][2]);
    trans.move_local(dp);

    return true;
  }
  return false;
}

bool RotationGizmo(::resin::Transform& trans, const ::resin::Camera& camera, GizmoMode mode, float width,
                   float height) {
  auto view      = camera.view_matrix();
  auto proj      = camera.proj_matrix();
  auto delta_mat = glm::mat4(1.0F);
  auto mat       = trans.local_to_world_matrix();

  ImGuizmo::BeginFrame();
  ImGuizmo::SetDrawlist();
  ImGuizmo::SetOrthographic(camera.is_orthographic);
  ImGuizmo::SetRect(ImGui::GetWindowPos().x + ImGui::GetCursorStartPos().x,
                    ImGui::GetWindowPos().y + ImGui::GetCursorStartPos().y, width, height);

  if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), ImGuizmo::OPERATION::ROTATE,
                           mode == GizmoMode::Local ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD,
                           glm::value_ptr(mat), glm::value_ptr(delta_mat))) {
    auto dq = glm::quat_cast(delta_mat);
    if (trans.has_parent()) {
      auto pq = trans.parent().rot();

      // new world rotation:
      //
      //      q_new = dq * pq * q,
      //
      // where q is the current local rotation of the object, pq is the parent
      // world rotation and dq is delta obtained from the ImGuizmo
      //
      // however we can only influence the q, so we need dq' that could be
      // plugged in like this:
      //
      //      q_new = pq * dq' * q
      //
      // from dq * pq * q = pq * dq' * q, we get
      //
      //      dq' = pq^{-1} * dq * pq

      dq = glm::normalize(glm::quat(pq.w, -pq.x, -pq.y, -pq.z) * dq * pq);
    }
    trans.rotate(dq);
    return true;
  }
  return false;
}

}  // namespace resin

}  // namespace ImGui
