#ifndef RESIN_CAMERA_HPP
#define RESIN_CAMERA_HPP
#include <libresin/core/transform.hpp>

namespace resin {

class Camera {
 public:
  float width;
  float height;
  float near_plane;
  float far_plane;
  Transform transform;
  bool is_orthographic;

  Camera(float width, float height, float near_plane, float far_plane, bool is_orthographic);
  virtual ~Camera();

  virtual glm::mat4 view_matrix() const;
  virtual glm::mat4 inverse_view_matrix() const;
};

class PerspectiveCamera : public Camera {
 public:
  float fov;
  float aspect_ratio;

  PerspectiveCamera(float fov, float aspect_ratio, float near_plane, float far_plane);

 private:
  void update_dimensions();
};

class OrthographicCamera : public Camera {
 public:
  OrthographicCamera(float width, float height, float near_plane, float far_plane);
};

}  // namespace resin

#endif  // RESIN_CAMERA_HPP
