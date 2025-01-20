#ifndef IMGUI_RESIN_GIZMO_HPP
#define IMGUI_RESIN_GIZMO_HPP

#include <imgui/imgui.h>

#include <cstdint>
#include <libresin/core/camera.hpp>
#include <libresin/core/transform.hpp>

namespace ImGui {  // NOLINT

namespace resin {

namespace gizmo {

enum class Mode : uint8_t { Local = 0, World = 1, _Count = 2 };                           // NOLINT
enum class Operation : uint8_t { Translation = 0, Rotation = 1, Scale = 2, _Count = 3 };  // NOLINT

void SetImGuiContext(ImGuiContext* ctx);
void BeginFrame(ImDrawList* drawlist = nullptr);
bool IsTransformUsed();
bool Transform(::resin::Transform& trans, const ::resin::Camera& camera, Mode mode, Operation operation,
               bool freeze = false);
bool CameraView(::resin::Camera& camera, float distance, float dt, bool freeze = false, float interpolation_time = 0.2F,
                ImVec2 size = ImVec2(128.0F, 128.0F));

}  // namespace gizmo

}  // namespace resin

}  // namespace ImGui

#endif
