#ifndef RESIN_TRANSFORM_GIZMO_HPP
#define RESIN_TRANSFORM_GIZMO_HPP

#include <cstdint>
#include <libresin/core/camera.hpp>
#include <libresin/core/transform.hpp>

namespace ImGui {  // NOLINT

namespace resin {

enum class GizmoMode : uint8_t { Local = 0, World };

bool TranslationGizmo(::resin::Transform& trans, const ::resin::Camera& camera, GizmoMode mode, float width,
                      float height);

bool RotationGizmo(::resin::Transform& trans, const ::resin::Camera& camera, GizmoMode mode, float width, float height);

}  // namespace resin

}  // namespace ImGui

#endif
