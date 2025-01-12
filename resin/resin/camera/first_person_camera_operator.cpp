#include <libresin/core/camera.hpp>
#include <resin/camera/first_person_camera_operator.hpp>

namespace resin {

FirstPersonCameraOperator::FirstPersonCameraOperator()
    : last_mouse_pos_(0.0F), is_active_(false), sensitivity_(0.08F), speed_(5.0F) {}

void FirstPersonCameraOperator::start_moving_in_dir(FirstPersonCameraOperator::Dir dir) {
  auto dir_ind = static_cast<size_t>(dir);
  if (dir_ind > kDirCount) {
    return;
  }
  move_dir_[dir_ind] = true;
}

void FirstPersonCameraOperator::stop_moving_in_dir(FirstPersonCameraOperator::Dir dir) {
  auto dir_ind = static_cast<size_t>(dir);
  if (dir_ind > kDirCount) {
    return;
  }
  move_dir_[dir_ind] = false;
}

bool FirstPersonCameraOperator::is_moving_in_dir(FirstPersonCameraOperator::Dir dir) {
  auto dir_ind = static_cast<size_t>(dir);
  if (dir_ind > kDirCount) {
    return false;
  }

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
    pos += camera.transform.local_front() * forward + camera.transform.local_right() * right + glm::vec3(0, 1, 0) * up;
    camera.transform.set_local_pos(pos);
    camera.recalculate_projection();

    glm::vec2 mouse_delta = (mouse_pos - last_mouse_pos_) * sensitivity_;
    auto rot              = camera.transform.local_rot();
    auto yaw              = glm::angleAxis(-mouse_delta.x * dt, glm::vec3(0, 1, 0));
    auto pitch            = glm::angleAxis(-mouse_delta.y * dt, camera.transform.local_right());
    auto new_rot          = yaw * pitch * rot;
    camera.transform.set_local_rot(new_rot);
  }

  last_mouse_pos_ = mouse_pos;
  return is_active_;
}

}  // namespace resin
