#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <libresin/core/camera.hpp>

namespace resin {

Camera::Camera(const bool orthographic, const float fov, const float aspect_ratio, const float near_plane,
               float far_plane)
    : is_orthographic(orthographic),
      fov_(fov),
      aspect_ratio_(aspect_ratio),
      near_plane_(near_plane),
      far_plane_(far_plane),
      width_(0.0F),
      height_(0.0F) {
  update_dimensions();
}

void Camera::set_fov(const float fov) {
  fov_ = fov;
  update_dimensions();
}

void Camera::set_aspect_ratio(const float aspect_ratio) {
  aspect_ratio_ = aspect_ratio;
  update_dimensions();
}

void Camera::set_near_plane(const float near_plane) {
  near_plane_ = near_plane;
  update_dimensions();
}

void Camera::set_far_plane(const float far_plane) { far_plane_ = far_plane; }

void Camera::update_dimensions() {
  height_ = 2.0F * near_plane_ * std::tan(glm::radians(fov_ * 0.5F));
  width_  = height_ * aspect_ratio_;
}

glm::mat4 Camera::view_matrix() const { return transform.world_to_local_matrix(); }
glm::mat4 Camera::inverse_view_matrix() const { return transform.local_to_world_matrix(); }

glm::mat4 Camera::proj_matrix() const {
  if (is_orthographic) {
    return glm::ortho(-width_ / 2.0F, width_ / 2.0F, -height_ / 2.0F, height_ / 2.0F, near_plane_, far_plane_);
  }

  return glm::perspective(glm::radians(fov_), aspect_ratio_, near_plane_, far_plane_);
}

}  // namespace resin
