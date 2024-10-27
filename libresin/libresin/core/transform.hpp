#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <optional>

namespace resin {

class transform {
 public:
  explicit transform(glm::vec3 pos = glm::vec3(), glm::quat rot = {1, 0, 0, 0},
                     glm::vec3 scale = {1, 1, 1});
  ~transform();

  // getters / setters
  const glm::vec3& local_pos() const { return pos_; }
  glm::vec3& local_pos() { return pos_; }

  const glm::quat& local_rot() const { return rot_; }
  glm::quat& local_rot() { return rot_; }

  const glm::vec3& local_scale() const { return scale_; }
  glm::vec3& local_scale() { return scale_; }

  transform(const transform&) = delete;
  transform(transform&&) = delete;
  transform& operator=(const transform&) = delete;
  transform& operator=(transform&&) = delete;

 private:
  // std::optional<std::reference_wrapper<transform>> parent_;
  glm::vec3 pos_;
  glm::quat rot_;
  glm::vec3 scale_;
};  // class transform

}  // namespace resin

#endif  // TRANSFORM_HPP
