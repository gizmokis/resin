#include <libresin/core/camera.hpp>
#include <resin/camera/orbiting_camera_operator.hpp>
#include <resin/resin.hpp>

namespace resin {

OrbitingCameraOperator::OrbitingCameraOperator()
    : last_mouse_pos_(0.0F), center_(0.0F), is_active_(false), sensitivity_(0.003F) {}

bool OrbitingCameraOperator::update(Camera& camera, float camera_distance, glm::vec2 mouse_pos) {
  if (is_active_) {
    glm::vec2 mouse_delta = (mouse_pos - last_mouse_pos_) * sensitivity_;
    Logger::info("{} {}", mouse_delta.x, mouse_delta.y);
    auto rot     = camera.transform.local_rot();
    auto yaw     = glm::angleAxis(-mouse_delta.x, glm::vec3(0, 1, 0));
    auto pitch   = glm::angleAxis(-mouse_delta.y, camera.transform.local_right());
    auto new_rot = yaw * pitch * rot;

    camera.transform.set_local_rot(new_rot);
    camera.transform.set_local_pos(-camera.transform.local_front() * camera_distance);
  }

  last_mouse_pos_ = mouse_pos;
  return is_active_;
}

}  // namespace resin
