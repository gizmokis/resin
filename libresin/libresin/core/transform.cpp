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

void Transform::look_at(const glm::vec3& point, const glm::vec3& up) {
  const glm::vec3 local_point =
      parent_.has_value() ? glm::vec3(parent().world_to_local_matrix() * glm::vec4(point, 1.0F)) : point;
  glm::vec3 direction = local_point - pos_;
  direction = glm::normalize(direction);
  rot_ = glm::quatLookAt(direction, up);
}

void Transform::fma(const glm::vec3& axis, const float amount) {
  pos_ += amount * axis;
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

void Transform::set_local_scale(const glm::vec3& scale) {
  scale_ = scale;
  mark_dirty();
}

void Transform::set_local_scale(const float scale) {
  scale_ = glm::vec3(scale);
  mark_dirty();
}

glm::vec3 Transform::front() const {
  const glm::vec3 local = local_front();
  return parent_ ? parent().local_to_world_matrix() * glm::vec4(local, 0.0F) : local;
}

glm::vec3 Transform::right() const {
  const glm::vec3 local = local_right();
  return parent_ ? parent().local_to_world_matrix() * glm::vec4(local, 0.0F) : local;
}

glm::mat4 Transform::local_to_parent_matrix() const {
  return glm::translate(pos_) * glm::mat4_cast(rot_) * glm::scale(scale_);
}

glm::mat4 Transform::parent_to_local_matrix() const {
  return glm::scale(1.0F / scale_) * glm::mat4_cast(glm::inverse(rot_)) * glm::translate(-pos_);
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

glm::vec3 Transform::up() const {
  const glm::vec3 local = local_up();
  return parent_ ? parent().local_to_world_matrix() * glm::vec4(local, 0.0F) : local;
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

void Transform::remove_from_parent() {
  std::erase_if(parent_->get().children_, [this](auto ref) { return std::addressof(ref.get()) == this; });

  parent_.reset();
}

}  // namespace resin
