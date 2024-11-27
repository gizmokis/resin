#ifndef RESIN_CAMERA_HPP
#define RESIN_CAMERA_HPP
#include <libresin/core/transform.hpp>

namespace resin {

class Camera {
 public:
  Transform transform;
  bool is_orthographic;

  Camera(bool orthographic, float fov, float aspect_ratio, float near_plane, float far_plane);

  float fov() const { return fov_; }
  float aspect_ratio() const { return aspect_ratio_; }
  float near_plane() const { return near_plane_; }
  float far_plane() const { return far_plane_; }
  float width() const { return width_; }
  float height() const { return height_; }

  void set_fov(float fov);
  void set_aspect_ratio(float aspect_ratio);
  void set_near_plane(float near_plane);
  void set_far_plane(float far_plane);

  glm::mat4 view_matrix() const;
  glm::mat4 inverse_view_matrix() const;

 private:
  float fov_;  // only apparent if orthographic
  float aspect_ratio_;
  float near_plane_;
  float far_plane_;
  float width_;
  float height_;

  void update_dimensions();
};

}  // namespace resin

#endif  // RESIN_CAMERA_HPP
