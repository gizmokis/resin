#ifndef RESIN_TRANSFORM_GIZMO_HPP
#define RESIN_TRANSFORM_GIZMO_HPP

#include <cstdint>
#include <libresin/core/camera.hpp>
#include <libresin/core/transform.hpp>

namespace ImGui {  // NOLINT

namespace resin {

enum class GizmoMode : uint8_t { Local = 0, World = 1, _Count = 2 };                           // NOLINT
enum class GizmoOperation : uint8_t { Translation = 0, Rotation = 1, Scale = 2, _Count = 3 };  // NOLINT

bool TransformGizmo(::resin::Transform& trans, const ::resin::Camera& camera, GizmoMode mode, GizmoOperation operation,
                    float width, float height);

}  // namespace resin

}  // namespace ImGui

#endif
