#ifndef RESIN_CAMERA_HPP
#define RESIN_CAMERA_HPP
#include <libresin/core/transform.hpp>

namespace resin {

class Camera {
 public:
  Transform transform;

  Camera(bool orthographic, float fov, float aspect_ratio, float near_plane, float far_plane);

  bool is_orthographic() const { return is_orthographic_; }
  float fov() const { return fov_; }
  float aspect_ratio() const { return aspect_ratio_; }
  float near_plane() const { return near_plane_; }
  float far_plane() const { return far_plane_; }
  float width() const { return width_; }
  float height() const { return height_; }

  void set_orthographic(bool ortho);
  void set_fov(float fov);
  void set_aspect_ratio(float aspect_ratio);
  void set_near_plane(float near_plane);
  void set_far_plane(float far_plane);

  glm::mat4 view_matrix() const { return transform.world_to_local_matrix(); }
  glm::mat4 inverse_view_matrix() const { return transform.local_to_world_matrix(); }
  glm::mat4 proj_matrix() const { return projection_; }

  void recalculate_projection();

 private:
  bool is_orthographic_;

  float fov_;  // only apparent if orthographic
  float aspect_ratio_;
  float width_, height_;
  float near_plane_, far_plane_;

  glm::mat4 projection_;
};

}  // namespace resin

#endif  // RESIN_CAMERA_HPP
