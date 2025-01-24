#ifndef RESIN_TRANSFORM_HPP
#define RESIN_TRANSFORM_HPP
#include <functional>
#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>
#include <libresin/core/id_registry.hpp>
#include <optional>

namespace resin {

struct Transform;
using TransformId = Id<Transform>;

struct Transform final {
 public:
  explicit Transform(const glm::vec3 pos = glm::vec3(), const glm::quat rot = {1, 0, 0, 0}, const float scale = 1)
      : pos_(pos), rot_(rot), scale_(scale), dirty_(true), inv_dirty_(true) {}
  ~Transform();

  bool has_parent() const { return parent_.has_value(); }
  const Transform& parent() const { return *parent_; }
  void set_parent(std::optional<std::reference_wrapper<Transform>> parent);

  void move(const glm::vec3& delta);
  void move_local(const glm::vec3& delta);

  void rotate(const glm::vec3& axis, float angle);
  void rotate(const glm::quat& rotation);
  void rotate_local(const glm::quat& rotation);

  const glm::vec3& local_pos() const { return pos_; }
  glm::vec3& local_pos() { return pos_; }
  glm::vec3 pos() const;
  void set_local_pos(const glm::vec3& pos);

  const glm::quat& local_rot() const { return rot_; }
  glm::quat& local_rot() { return rot_; }
  glm::quat rot() const;
  void set_local_rot(const glm::quat& rot);

  const float& local_scale() const { return scale_; }
  float& local_scale() { return scale_; }
  float scale() const;
  void set_local_scale(float scale);

  void set_local_from_matrix(const glm::mat4& local_mat);

  glm::mat3 local_orientation() const;
  glm::mat3 orientation() const;
  glm::vec3 local_front() const { return rot_ * glm::vec3(0, 0, -1); }
  glm::vec3 front() const;
  glm::vec3 local_right() const { return rot_ * glm::vec3(1, 0, 0); }
  glm::vec3 right() const;
  glm::vec3 local_up() const { return rot_ * glm::vec3(0, 1, 0); }
  glm::vec3 up() const;

  void mark_dirty() const;
  glm::mat4 local_to_parent_matrix() const;
  glm::mat4 parent_to_local_matrix() const;
  const glm::mat4& local_to_world_matrix() const;
  const glm::mat4& world_to_local_matrix() const;

  Transform(const Transform&)            = delete;
  Transform(Transform&&)                 = delete;
  Transform& operator=(const Transform&) = delete;
  Transform& operator=(Transform&&)      = delete;

  void shallow_copy(const Transform& otherl);

  void remove_parent();

 private:
  std::optional<std::reference_wrapper<Transform>> parent_;
  std::vector<std::reference_wrapper<Transform>> children_;

  glm::vec3 pos_;
  glm::quat rot_;
  float scale_;

  mutable bool dirty_              = false;
  mutable glm::mat4 model_mat_     = glm::mat4(1.0F);
  mutable bool inv_dirty_          = false;
  mutable glm::mat4 inv_model_mat_ = glm::mat4(1.0F);
};  // class Transform

}  // namespace resin
#endif  // RESIN_TRANSFORM_HPP
