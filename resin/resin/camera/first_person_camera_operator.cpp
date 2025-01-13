#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <libresin/core/camera.hpp>
#include <resin/camera/first_person_camera_operator.hpp>
#include <resin/core/key_codes.hpp>

namespace resin {

FirstPersonCameraOperator::FirstPersonCameraOperator()
    : last_mouse_pos_(0.0F), is_active_(false), use_local_axises_(false), sensitivity_(0.08F), speed_(5.0F) {}

void FirstPersonCameraOperator::start_move(key::Code key_code) {
  auto it = key_mappings_.find(key_code);
  if (it != key_mappings_.end()) {
    move_dir(it->second) = true;
  }
}

void FirstPersonCameraOperator::stop_move(key::Code key_code) {
  auto it = key_mappings_.find(key_code);
  if (it != key_mappings_.end()) {
    move_dir(it->second) = false;
  }
}

bool FirstPersonCameraOperator::is_moving_in_dir(FirstPersonCameraOperator::Dir dir) {
  auto dir_ind = static_cast<size_t>(dir);
  if (dir_ind > kDirCount) {
    return false;
  }

  return move_dir_[dir_ind];
}

bool& FirstPersonCameraOperator::move_dir(FirstPersonCameraOperator::Dir dir) {
  auto dir_ind = static_cast<size_t>(dir);
  return move_dir_[dir_ind];
}

bool FirstPersonCameraOperator::update(Camera& camera, glm::vec2 mouse_pos, float dt) {
  if (is_active_) {
    float right   = 0.F;
    float up      = 0.F;
    float forward = 0.F;

    if (!camera.is_orthographic()) {
      if (is_moving_in_dir(Dir::Front)) {
        forward += speed_ * dt;
      }
      if (is_moving_in_dir(Dir::Back)) {
        forward -= speed_ * dt;
      }
    }

    if (is_moving_in_dir(Dir::Left)) {
      right -= speed_ * dt;
    }
    if (is_moving_in_dir(Dir::Right)) {
      right += speed_ * dt;
    }

    if (is_moving_in_dir(Dir::Up)) {
      up -= speed_ * dt;
    }
    if (is_moving_in_dir(Dir::Down)) {
      up += speed_ * dt;
    }

    auto pos = camera.transform.local_pos();
    pos += (use_local_axises_
                ? camera.transform.local_front()
                : glm::normalize(glm::vec3(camera.transform.local_front().x, 0.0F, camera.transform.local_front().z))) *
               forward +
           camera.transform.local_right() * right +
           (use_local_axises_ ? camera.transform.local_up() : glm::vec3(0.0F, 1.0F, 0.0F)) * up;
    camera.transform.set_local_pos(pos);
    camera.recalculate_projection();

    glm::vec2 mouse_delta = (mouse_pos - last_mouse_pos_) * sensitivity_;
    auto rot              = camera.transform.local_rot();
    auto yaw              = glm::angleAxis(-mouse_delta.x * dt, glm::vec3(0, 1, 0));
    auto pitch            = glm::angleAxis(-mouse_delta.y * dt, camera.transform.local_right());
    auto new_rot          = yaw * pitch * rot;

    // Prevent 360 flips
    auto new_up = new_rot * glm::vec3(0.0F, 1.0F, 0.0F);
    if (new_up.y < 0.0F) {
      auto new_dir = new_rot * glm::vec3(0.0F, 0.0F, -1.0F);
      if (new_dir.y > 0.0F) {
        // operator looks up
        new_rot = glm::quatLookAt(glm::vec3(0.0F, 1.0F, 0.0F), new_up);
      } else {
        // operator looks down
        new_rot = glm::quatLookAt(glm::vec3(0.0F, -1.0F, 0.0F), new_up);
      }
    }
    camera.transform.set_local_rot(new_rot);
  }

  last_mouse_pos_ = mouse_pos;
  return is_active_;
}

}  // namespace resin
