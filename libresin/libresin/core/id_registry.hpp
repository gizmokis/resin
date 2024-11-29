#ifndef RESIN_COMPONENTS_HPP
#define RESIN_COMPONENTS_HPP
#include <functional>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>
#include <mutex>
#include <stack>

namespace resin {

// Thread safe id manager.
template <typename Obj>
class IdRegistry {
 public:
  IdRegistry() = delete;

  explicit IdRegistry(size_t max_objs) : max_objs_(max_objs) {
    for (size_t i = max_objs_; i-- > 0;) {
      freed_.push(i);
    }

    is_registered_.resize(max_objs_, false);
  }

  size_t register_id() {
    const std::lock_guard lock(mutex_);

    if (freed_.empty()) {
      log_throw(ObjectsOverflowException());
    }

    size_t new_id = freed_.top();
    freed_.pop();
    is_registered_[new_id] = true;

    return new_id;
  }

  bool is_registered(size_t id) const {
    const std::lock_guard lock(mutex_);

    if (id >= max_objs_) {
      return false;
    }

    return is_registered_[id];
  }

  size_t get_max_objs() const { return max_objs_; }

  void unregister_id(size_t id) {
    const std::lock_guard lock(mutex_);

    if (id >= max_objs_) {
      Logger::warn("Detected an attempt to unregister a non-existent id [{}].", id);
      return;
    }

    is_registered_[id] = false;
    freed_.push(id);
  }

  IdRegistry(const IdRegistry&)            = delete;
  IdRegistry& operator=(const IdRegistry&) = delete;

  ~IdRegistry() {
    if (freed_.size() != max_objs_) {
      Logger::err("Id registry is being destructed but not all ids are unregistered!");
    }
  }

 private:
  std::stack<size_t> freed_;
  std::vector<bool> is_registered_;
  size_t max_objs_;
  std::mutex mutex_;
};

// Strongly typed id, with similar behavior to unique_ptr -- it unregisters in the provided registry when destructor is
// called.
template <typename Obj>
struct Id {
 public:
  using object_type = Obj;

  Id() = delete;
  // It is a programmer responsibility to assert that Id class will not outlive the provided registry!
  explicit Id(IdRegistry<Obj>& registry) : registry_(registry) { raw_id_ = registry_.get().register_id(); }

  ~Id() { registry_.get().unregister_id(raw_id_); }

  Id(const Id<Obj>& other)                 = delete;
  Id<Obj>& operator=(const Id<Obj>& other) = delete;

  bool operator==(const Id<Obj>& other) const { return raw_id_ == other.raw(); }
  bool operator!=(const Id<Obj>& other) const { return raw_id_ != other.raw(); }

  inline size_t raw() const { return raw_id_; }
  inline int raw_as_int() const { return static_cast<int>(raw_id_); }

  std::reference_wrapper<const IdRegistry<Obj>> registry() const { return registry_; }

 private:
  size_t raw_id_;
  std::reference_wrapper<IdRegistry<Obj>> registry_;
};

// Strongly typed id observer. It does not ensure that the observed Id is still registered.
template <typename IdType>
  requires std::is_base_of_v<Id<typename IdType::object_type>, IdType>
struct IdView {
  IdView() = delete;
  IdView(const IdType& id)  // NOLINT (allow the implicit constructor)
      : raw_id_(id.raw()) {}

  bool operator==(const IdView<IdType>& other) const { return raw_id_ == other.raw(); }
  bool operator!=(const IdView<IdType>& other) const { return raw_id_ != other.raw(); }
  bool operator==(const IdType& other) const { return raw_id_ == other.raw(); }
  bool operator!=(const IdType& other) const { return raw_id_ != other.raw(); }

  inline size_t raw() const { return raw_id_; }
  inline int raw_as_int() const { return static_cast<int>(raw_id_); }

 private:
  size_t raw_id_;
};

}  // namespace resin

#endif
