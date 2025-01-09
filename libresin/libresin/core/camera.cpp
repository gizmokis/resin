#include <glm/ext/matrix_clip_space.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <libresin/core/camera.hpp>

namespace resin {

Camera::Camera(bool orthographic, float fov, float aspect_ratio, float near_plane, float far_plane)
    : is_orthographic_(orthographic),
      fov_(fov),
      aspect_ratio_(aspect_ratio),
      width_(),
      height_(),
      near_plane_(near_plane),
      far_plane_(far_plane),
      projection_() {
  recalculate_projection();
}

void Camera::set_orthographic(bool ortho) {
  is_orthographic_ = ortho;
  recalculate_projection();
}

void Camera::set_fov(float fov) {
  fov_ = fov;
  recalculate_projection();
}

void Camera::set_aspect_ratio(float aspect_ratio) {
  aspect_ratio_ = aspect_ratio;
  recalculate_projection();
}

void Camera::set_near_plane(float near_plane) {
  near_plane_ = near_plane;
  recalculate_projection();
}

void Camera::set_far_plane(float far_plane) { far_plane_ = far_plane; }

void Camera::recalculate_projection() {
  float focal_length = is_orthographic_ ? glm::length(transform.pos()) : near_plane_;
  height_            = focal_length * std::tan(glm::radians(fov_ * 0.5F));
  width_             = height_ * aspect_ratio_;

  if (is_orthographic_) {
    projection_ = glm::ortho(-width_, width_, -height_, height_, near_plane_, far_plane_);
  } else {
    projection_ = glm::perspective(glm::radians(fov_), aspect_ratio_, near_plane_, far_plane_);
  }
}

}  // namespace resin
