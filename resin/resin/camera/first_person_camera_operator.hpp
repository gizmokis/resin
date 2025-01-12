#ifndef RESIN_FIRST_PERSON_CAMERA_OPERATOR_HPP
#define RESIN_FIRST_PERSON_CAMERA_OPERATOR_HPP

#include <array>
#include <cstdint>
#include <glm/vec2.hpp>

namespace resin {

class Camera;

class FirstPersonCameraOperator {
 public:
  FirstPersonCameraOperator();

  FirstPersonCameraOperator(const FirstPersonCameraOperator&)            = delete;
  FirstPersonCameraOperator(FirstPersonCameraOperator&&)                 = delete;
  FirstPersonCameraOperator& operator=(const FirstPersonCameraOperator&) = delete;
  FirstPersonCameraOperator& operator=(FirstPersonCameraOperator&&)      = delete;

  enum class Dir : uint8_t {
    Front,
    Back,
    Left,
    Right,
    Up,
    Down,
    _Count  // NOLINT
  };

  static constexpr size_t kDirCount = static_cast<size_t>(Dir::_Count);

  inline void start() { is_active_ = true; }
  inline void stop() { is_active_ = false; }
  void start_moving_in_dir(Dir dir);
  void stop_moving_in_dir(Dir dir);
  bool is_moving_in_dir(Dir dir);

  bool update(Camera& camera, glm::vec2 mouse_pos, float dt);
  inline void set_sensitivity(float sensitivity) { sensitivity_ = sensitivity; }
  inline void set_speed(float speed) { speed_ = speed; }
  inline bool is_active() const { return is_active_; }

 private:
 private:
  std::array<bool, kDirCount> move_dir_{};
  glm::vec2 last_mouse_pos_;
  bool is_active_;
  float sensitivity_;
  float speed_;
};

}  // namespace resin

#endif
