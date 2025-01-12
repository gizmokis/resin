#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imguizmo/ImGuizmo.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>
#include <libresin/core/transform.hpp>
#include <resin/imgui/gizmo.hpp>

namespace ImGui {  // NOLINT

namespace resin {
namespace gizmo {

void SetImGuiContext(ImGuiContext* ctx) { ImGuizmo::SetImGuiContext(ctx); }

void BeginFrame(ImDrawList* drawlist) {
  ImGuizmo::BeginFrame();
  ImGuizmo::SetDrawlist(drawlist);
}

bool IsTransformUsed() { return ImGuizmo::IsUsing(); }

bool Transform(::resin::Transform& trans, const ::resin::Camera& camera, Mode mode, Operation operation, bool freeze) {
  const float pos_x  = ImGui::GetWindowPos().x + ImGui::GetCursorStartPos().x;
  const float pos_y  = ImGui::GetWindowPos().y + ImGui::GetCursorStartPos().y;
  const float width  = ImGui::GetWindowWidth() - ImGui::GetCursorStartPos().x;
  const float height = ImGui::GetWindowHeight() - ImGui::GetCursorStartPos().y;

  ImGuizmo::SetOrthographic(camera.is_orthographic());
  ImGuizmo::SetRect(pos_x, pos_y, width, height);

  auto view      = camera.view_matrix();
  auto proj      = camera.proj_matrix();
  auto delta_mat = glm::mat4(1.0F);
  auto mat       = trans.local_to_world_matrix();

  if (operation == Operation::Translation) {
    if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), ImGuizmo::OPERATION::TRANSLATE,
                             mode == Mode::Local ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD, glm::value_ptr(mat),
                             glm::value_ptr(delta_mat)) &&
        !freeze) {
      auto dp = glm::vec3(delta_mat[3]);
      if (trans.has_parent()) {
        auto parent_rot_mat = glm::mat4_cast(trans.parent().rot());
        dp                  = glm::vec3(glm::transpose(parent_rot_mat) * glm::vec4(dp, 1.0F));
      }
      trans.move(dp);

      return true;
    }
    return false;
  }

  if (operation == Operation::Rotation) {
    if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), ImGuizmo::OPERATION::ROTATE,
                             mode == Mode::Local ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD, glm::value_ptr(mat),
                             glm::value_ptr(delta_mat)) &&
        !freeze) {
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

        dq = glm::normalize(glm::inverse(pq) * dq * pq);
      }
      trans.rotate(dq);
      return true;
    }
    return false;
  }

  if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), ImGuizmo::OPERATION::SCALE,
                           ImGuizmo::MODE::WORLD, glm::value_ptr(mat), glm::value_ptr(delta_mat)) &&
      !freeze) {
    constexpr float kFloatEqTreshold = 1e-5F;

    // Note: delta matrix doesn't contain a delta per frame, but a delta from the moment the user grabbed the gizmo
    // for the first time so we can't use it here ☹️
    float scale_fix = 1.0F;
    if (trans.has_parent()) {
      scale_fix = trans.parent().scale();
    }

    // Assuming uniform scaling
    if (std::abs(mat[1][1] - mat[0][0]) > kFloatEqTreshold) {
      if (std::abs(mat[1][1] - mat[2][2]) > kFloatEqTreshold) {
        trans.set_local_scale(mat[1][1] / scale_fix);
      } else {
        trans.set_local_scale(mat[0][0] / scale_fix);
      }
    } else if (std::abs(mat[2][2] - mat[0][0]) > kFloatEqTreshold) {
      trans.set_local_scale(mat[2][2] / scale_fix);
    } else if (std::abs(mat[0][0] - trans.scale()) > kFloatEqTreshold) {
      // handle the case when origin is used for scaling
      trans.set_local_scale(mat[0][0] / scale_fix);
    }

    return true;
  }

  return false;
}

bool CameraView(::resin::Camera& camera, float distance, float dt, bool freeze, float interpolation_time, ImVec2 size) {
  const float pos_x = ImGui::GetWindowPos().x + ImGui::GetCursorStartPos().x;
  const float pos_y = ImGui::GetWindowPos().y + ImGui::GetCursorStartPos().y;
  const float width = ImGui::GetWindowWidth() - ImGui::GetCursorStartPos().x;

  auto inv_view_rot = camera.transform.local_rot();
  auto inv_view_pos = camera.transform.local_pos();
  if (ImGuizmo::ViewManipulate(camera.view_matrix(), inv_view_pos, inv_view_rot, distance,
                               ImVec2(pos_x + width - size.x, pos_y), size, 0x00000000, false, dt,
                               interpolation_time) &&
      !freeze) {
    camera.transform.set_local_pos(inv_view_pos);
    camera.transform.set_local_rot(inv_view_rot);
    return true;
  }
  return false;
}

}  // namespace gizmo

}  // namespace resin

}  // namespace ImGui
