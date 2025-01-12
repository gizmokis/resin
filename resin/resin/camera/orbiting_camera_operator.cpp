#include <libresin/core/camera.hpp>
#include <resin/camera/orbiting_camera_operator.hpp>

namespace resin {

OrbitingCameraOperator::OrbitingCameraOperator()
    : last_mouse_pos_(0.0F), center_(0.0F), is_active_(false), sensitivity_(0.08F) {}

bool OrbitingCameraOperator::update(Camera& camera, float camera_distance, glm::vec2 mouse_pos, float dt) {
  if (is_active_) {
    glm::vec2 mouse_delta = (mouse_pos - last_mouse_pos_) * sensitivity_;
    auto rot              = camera.transform.local_rot();
    auto yaw              = glm::angleAxis(-mouse_delta.x * dt, glm::vec3(0, 1, 0));
    auto pitch            = glm::angleAxis(-mouse_delta.y * dt, camera.transform.local_right());
    auto new_rot          = yaw * pitch * rot;

    camera.transform.set_local_rot(new_rot);
    camera.transform.set_local_pos(center_ - camera.transform.local_front() * camera_distance);
    camera.recalculate_projection();
  }

  last_mouse_pos_ = mouse_pos;
  return is_active_;
}

}  // namespace resin
