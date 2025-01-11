#ifndef RESIN_GIZMO_HPP
#define RESIN_GIZMO_HPP

#include <imgui/imgui.h>

#include <cstdint>
#include <libresin/core/camera.hpp>
#include <libresin/core/transform.hpp>

namespace ImGui {  // NOLINT

namespace resin {

enum class GizmoMode : uint8_t { Local = 0, World = 1, _Count = 2 };                           // NOLINT
enum class GizmoOperation : uint8_t { Translation = 0, Rotation = 1, Scale = 2, _Count = 3 };  // NOLINT

void SetImGuiContext(ImGuiContext* ctx);
void BeginGizmoFrame(ImDrawList* drawlist = nullptr);
bool TransformGizmo(::resin::Transform& trans, const ::resin::Camera& camera, GizmoMode mode, GizmoOperation operation);
bool CameraViewGizmo(::resin::Camera& camera, float distance, float dt, ImVec2 size = ImVec2(128.0F, 128.0F));

}  // namespace resin

}  // namespace ImGui

#endif
