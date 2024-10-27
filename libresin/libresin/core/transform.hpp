#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP
#define GLM_ENABLE_EXPERIMENTAL
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <optional>

namespace resin {

struct transform {
 public:
  explicit transform(const glm::vec3 pos = glm::vec3(),
                     const glm::quat rot = {1, 0, 0, 0},
                     const glm::vec3 scale = {1, 1, 1})
      : pos_(pos), rot_(rot), scale_(scale), dirty_(true), inv_dirty_(true) {}
  ~transform();

  bool has_parent() const { return parent_.has_value(); }
  const transform& parent() const { return *parent_; }
  void set_parent(std::optional<std::reference_wrapper<transform>> parent);

  void rotate(const glm::vec3& axis, float angle);
  void rotate(const glm::quat& rotation);
  void rotate_local(const glm::quat& rotation);
  void look_at(const glm::vec3& point,
               const glm::vec3& up = glm::vec3(0, 1, 0));
  void fma(const glm::vec3& axis, float amount);

  const glm::vec3& local_pos() const { return pos_; }
  glm::vec3& local_pos() { return pos_; }
  glm::vec3 pos() const;
  void set_local_pos(const glm::vec3& pos);

  const glm::quat& local_rot() const { return rot_; }
  glm::quat& local_rot() { return rot_; }
  glm::quat rot() const;
  void set_local_rot(const glm::quat& rot);

  const glm::vec3& local_scale() const { return scale_; }
  glm::vec3& local_scale() { return scale_; }
  void set_local_scale(const glm::vec3& scale);
  void set_local_scale(float scale);

  glm::vec3 local_front() const { return rot_ * glm::vec3(0, 0, -1); }
  glm::vec3 front() const;
  glm::vec3 local_right() const { return rot_ * glm::vec3(1, 0, 0); }
  glm::vec3 right() const;
  glm::vec3 local_up() const { return rot_ * glm::vec3(0, 1, 0); }
  glm::vec3 up() const;

  glm::mat4 local_to_parent_matrix() const;
  glm::mat4 parent_to_local_matrix() const;
  const glm::mat4& local_to_world_matrix() const;
  const glm::mat4& world_to_local_matrix() const;

  transform(const transform&) = delete;
  transform(transform&&) = delete;
  transform& operator=(const transform&) = delete;
  transform& operator=(transform&&) = delete;

 protected:
  void mark_dirty() const;

 private:
  void clear_parent();

 protected:
  std::optional<std::reference_wrapper<transform>> parent_;
  std::vector<std::reference_wrapper<transform>> children_;

 private:
  glm::vec3 pos_;
  glm::quat rot_;
  glm::vec3 scale_;

  mutable bool dirty_ = false;
  mutable glm::mat4 model_mat_ = glm::mat4(1.0f);
  mutable bool inv_dirty_ = false;
  mutable glm::mat4 inv_model_mat_ = glm::mat4(1.0f);
};  // class transform
}  // namespace resin
#endif  // TRANSFORM_HPP
