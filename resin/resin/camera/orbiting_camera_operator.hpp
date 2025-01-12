#ifndef RESIN_ORBITING_CAMERA_OPERATOR_HPP
#define RESIN_ORBITING_CAMERA_OPERATOR_HPP

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace resin {

class Camera;

class OrbitingCameraOperator {
 public:
  OrbitingCameraOperator();

  OrbitingCameraOperator(const OrbitingCameraOperator&)            = delete;
  OrbitingCameraOperator(OrbitingCameraOperator&&)                 = delete;
  OrbitingCameraOperator& operator=(const OrbitingCameraOperator&) = delete;
  OrbitingCameraOperator& operator=(OrbitingCameraOperator&&)      = delete;

  inline void start() { is_active_ = true; }
  inline void stop() { is_active_ = false; }
  bool update(Camera& camera, float camera_distance, glm::vec2 mouse_pos, float dt);
  inline void set_sensitivity(float sensitivity) { sensitivity_ = sensitivity; }
  inline void set_center(glm::vec3 center) { center_ = center; }
  inline bool is_active() const { return is_active_; }

 private:
  glm::vec2 last_mouse_pos_;
  glm::vec3 center_;
  bool is_active_;
  float sensitivity_;
};

}  // namespace resin

#endif
