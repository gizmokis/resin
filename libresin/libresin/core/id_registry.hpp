#ifndef RESIN_COMPONENTS_HPP
#define RESIN_COMPONENTS_HPP
#include <array>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>
#include <stack>

namespace resin {

template <typename Obj>
class IdRegistry;

template <typename Obj>
struct TypedId {
  bool operator==(const TypedId<Obj>& other) const { return raw_id_ == other.raw_id_; }
  bool operator!=(const TypedId<Obj>& other) const { return raw_id_ != other.raw_id_; }

  inline size_t get_raw() const { return raw_id_; }

 private:
  friend IdRegistry<Obj>;

  explicit TypedId(size_t value) : raw_id_(value) {}
  size_t raw_id_;
};

template <typename Obj>
class IdRegistry {
 public:
  static const size_t kMaxObjects = 8000;

  static IdRegistry<Obj>& get_instance() {
    static IdRegistry<Obj> instance;
    return instance;
  }

  TypedId<Obj> register_id() {
    if (freed_.empty()) {
      log_throw(ObjectsOverflowException());
    }

    size_t new_id = freed_.top();
    freed_.pop();
    is_registered_[new_id] = true;

    return TypedId<Obj>(new_id);
  }

  bool is_registered(TypedId<Obj> id) {
    if (id.get_raw() >= kMaxObjects) {
      return false;
    }

    return is_registered_[id.get_raw()];
  }

  void unregister_id(TypedId<Obj> id) {
    if (id.get_raw() >= kMaxObjects) {
      Logger::warn("Detected an attempt to unregister a non-existent id [{}].", id.get_raw());
      return;
    }

    is_registered_[id.get_raw()] = false;
    freed_.push(id.get_raw());
  }

  IdRegistry(const IdRegistry&)            = delete;
  IdRegistry& operator=(const IdRegistry&) = delete;

 private:
  IdRegistry() {
    for (size_t i = kMaxObjects - 1; i >= 0; --i) {
      freed_.push(i);
    }
    std::fill(is_registered_.begin(), is_registered_.end(), false);
  }

  std::stack<size_t> freed_;
  std::array<bool, kMaxObjects> is_registered_{};
};

}  // namespace resin

#endif
