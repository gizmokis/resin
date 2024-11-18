#include <libresin/core/camera.hpp>

namespace resin {

// Abstract
Camera::Camera(float width, float height, float near_plane, float far_plane, bool is_orthographic)
    : width(width), height(height), near_plane(near_plane), far_plane(far_plane), is_orthographic(is_orthographic) {}

Camera::~Camera() {}
glm::mat4 Camera::view_matrix() const { return transform.world_to_local_matrix(); }
glm::mat4 Camera::inverse_view_matrix() const { return transform.local_to_world_matrix(); }

// Perspective
PerspectiveCamera::PerspectiveCamera(float fov, float aspect_ratio, float near_plane, float far_plane)
    : Camera(0.0f, 0.0f, near_plane, far_plane, false), fov(fov), aspect_ratio(aspect_ratio) {
  update_dimensions();
}

void PerspectiveCamera::update_dimensions() {
  height = 2.0f * near_plane * glm::tan(glm::radians(fov) * 0.5f);
  width  = height * aspect_ratio;
}

// Orthographic
OrthographicCamera::OrthographicCamera(float width, float height, float near_plane, float far_plane)
    : Camera(width, height, near_plane, far_plane, true) {}

}  // namespace resin