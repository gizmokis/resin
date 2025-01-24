#include <glm/geometric.hpp>
#include <glm/gtx/transform.hpp>
#include <libresin/core/transform.hpp>

namespace resin {

Transform::~Transform() {
  for (const auto child : children_) {
    child.get().parent_.reset();
  }

  if (parent_.has_value()) {
    remove_from_parent();
  }
}

void Transform::set_parent(const std::optional<std::reference_wrapper<Transform>> parent) {
  // clear reference from old parent
  if (parent_.has_value()) {
    remove_from_parent();
  }

  mark_dirty();
  if (!parent.has_value()) {
    return;
  }

  // setup reference in new parent
  parent_ = parent;
  parent_->get().children_.emplace_back(*this);
}

void Transform::move(const glm::vec3& delta) {
  pos_ += delta;
  mark_dirty();
}

void Transform::move_local(const glm::vec3& delta) {
  pos_ += delta.x * local_right() + delta.y * local_up() + delta.z * local_front();
  mark_dirty();
}

void Transform::rotate(const glm::vec3& axis, const float angle) {
  rot_ = glm::rotate(rot_, angle, axis);
  mark_dirty();
}

void Transform::rotate(const glm::quat& rotation) {
  rot_ = glm::normalize(rotation * rot_);
  mark_dirty();
}

void Transform::rotate_local(const glm::quat& rotation) {
  rot_ = glm::normalize(rot_ * rotation);
  mark_dirty();
}

void Transform::set_local_from_matrix(const glm::mat4& local_mat) {
  auto new_loc_pos    = glm::vec3(local_mat[3]);
  auto new_loc_rot    = glm::normalize(glm::quat_cast(local_mat));
  float new_loc_scale = glm::length(glm::vec3(local_mat[0][0], local_mat[1][0], local_mat[2][0]));

  pos_   = new_loc_pos;
  rot_   = new_loc_rot;
  scale_ = new_loc_scale;

  mark_dirty();
}

glm::vec3 Transform::pos() const {
  return parent_.has_value() ? glm::vec3(parent().local_to_world_matrix() * glm::vec4(pos_, 1.0F)) : pos_;
}

void Transform::set_local_pos(const glm::vec3& pos) {
  pos_ = pos;
  mark_dirty();
}

glm::quat Transform::rot() const { return parent_.has_value() ? parent().rot() * rot_ : rot_; }

void Transform::set_local_rot(const glm::quat& rot) {
  rot_ = rot;
  mark_dirty();
}

float Transform::scale() const { return parent_.has_value() ? parent().scale() * scale_ : scale_; }

void Transform::set_local_scale(float scale) {
  scale_ = scale;
  mark_dirty();
}

glm::mat3 Transform::local_orientation() const {
  const glm::mat3 local = glm::toMat3(rot_);
  return glm::mat3(local[0], local[1], -local[2]);
}

glm::mat3 Transform::orientation() const {
  const glm::mat3 local = local_orientation();
  if (!parent_) {
    return local;
  }
  const glm::mat3 orientation = glm::mat3(parent().local_to_world_matrix()) * local;
  return glm::mat3(glm::normalize(orientation[0]), glm::normalize(orientation[1]), glm::normalize(orientation[2]));
}

glm::vec3 Transform::front() const {
  const glm::vec3 local = local_front();
  return parent_ ? glm::normalize(parent().local_to_world_matrix() * glm::vec4(local, 0.0F)) : local;
}

glm::vec3 Transform::right() const {
  const glm::vec3 local = local_right();
  return parent_ ? glm::normalize(parent().local_to_world_matrix() * glm::vec4(local, 0.0F)) : local;
}

glm::vec3 Transform::up() const {
  const glm::vec3 local = local_up();
  return parent_ ? glm::normalize(parent().local_to_world_matrix() * glm::vec4(local, 0.0F)) : local;
}

glm::mat4 Transform::local_to_parent_matrix() const {
  return glm::translate(pos_) * glm::mat4_cast(rot_) * glm::scale(glm::vec3(scale_));
}

glm::mat4 Transform::parent_to_local_matrix() const {
  return glm::scale(glm::vec3(scale_ < 1e-6F ? 0.0F : 1.0F / scale_)) * glm::mat4_cast(glm::inverse(rot_)) *
         glm::translate(-pos_);
}

const glm::mat4& Transform::local_to_world_matrix() const {
  if (!dirty_) {
    return model_mat_;
  }

  model_mat_ = local_to_parent_matrix();
  if (parent_) {
    model_mat_ = parent_->get().local_to_world_matrix() * model_mat_;
  }

  dirty_ = false;
  return model_mat_;
}

const glm::mat4& Transform::world_to_local_matrix() const {
  if (!inv_dirty_) {
    return inv_model_mat_;
  }

  inv_model_mat_ = parent_to_local_matrix();
  if (parent_) {
    inv_model_mat_ *= parent_->get().world_to_local_matrix();
  }

  inv_dirty_ = false;
  return inv_model_mat_;
}

void Transform::mark_dirty() const {
  if (dirty_ && inv_dirty_) {
    return;
  }

  dirty_ = inv_dirty_ = true;
  for (const auto child : children_) {
    child.get().mark_dirty();
  }
}

void Transform::shallow_copy(const Transform& other) {
  pos_   = other.pos_;
  rot_   = other.rot_;
  scale_ = other.scale_;
}

void Transform::remove_from_parent() {
  std::erase_if(parent_->get().children_, [this](auto ref) { return std::addressof(ref.get()) == this; });

  parent_.reset();
}

}  // namespace resin
