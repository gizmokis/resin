#ifndef RESIN_COMPONENTS_HPP
#define RESIN_COMPONENTS_HPP
#include <array>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>
#include <mutex>
#include <stack>

namespace resin {

const size_t kDefaultMaxObjects = 5000;

template <typename Obj, size_t MaxObjects = kDefaultMaxObjects>
class ComponentIdRegistry {
 public:
  static ComponentIdRegistry<Obj, MaxObjects>& instance() {
    // thread-safe according to
    // https://stackoverflow.com/questions/1661529/is-meyers-implementation-of-the-singleton-pattern-thread-safe

    // maybe some dynamic max objs loading?
    static ComponentIdRegistry<Obj, MaxObjects> instance;
    return instance;
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

  bool is_registered(size_t id) {
    const std::lock_guard lock(mutex_);

    if (id >= MaxObjects) {
      return false;
    }

    return is_registered_[id];
  }

  void unregister_id(size_t id) {
    const std::lock_guard lock(mutex_);

    if (id >= MaxObjects) {
      Logger::warn("Detected an attempt to unregister a non-existent id [{}].", id);
      return;
    }

    is_registered_[id] = false;
    freed_.push(id);
  }

  ComponentIdRegistry(const ComponentIdRegistry&)            = delete;
  ComponentIdRegistry& operator=(const ComponentIdRegistry&) = delete;

 private:
  ComponentIdRegistry() {
    for (size_t i = MaxObjects; i-- > 0;) {
      freed_.push(i);
    }
    std::fill(is_registered_.begin(), is_registered_.end(), false);
  }

  std::stack<size_t> freed_;
  std::array<bool, MaxObjects> is_registered_{};
  std::mutex mutex_;
};

template <typename Obj, size_t MaxObjects = kDefaultMaxObjects>
struct ComponentId {
 public:
  using object_type                   = Obj;
  static constexpr size_t kMaxObjects = MaxObjects;

  ComponentId() : raw_id_(resin::ComponentIdRegistry<Obj, MaxObjects>::instance().register_id()) {}
  ~ComponentId() { resin::ComponentIdRegistry<Obj, MaxObjects>::instance().unregister_id(raw_id_); }

  ComponentId(const ComponentId<Obj, MaxObjects>& other)                             = delete;
  ComponentId<Obj, MaxObjects>& operator=(const ComponentId<Obj, MaxObjects>& other) = delete;

  bool operator==(const ComponentId<Obj, MaxObjects>& other) const { return raw_id_ == other.raw(); }
  bool operator!=(const ComponentId<Obj, MaxObjects>& other) const { return raw_id_ != other.raw(); }

  inline size_t raw() const { return raw_id_; }
  inline int raw_as_int() const { return static_cast<int>(raw_id_); }

 private:
  size_t raw_id_;
};

template <typename IdType>
  requires std::is_base_of_v<ComponentId<typename IdType::object_type, IdType::kMaxObjects>, IdType>
struct ComponentIdView {
  ComponentIdView() = delete;
  ComponentIdView(const IdType& id) : raw_id_(id.raw()) {}  // NOLINT (allow the implicit constructor)

  bool operator==(const ComponentIdView<IdType>& other) const { return raw_id_ == other.raw(); }
  bool operator!=(const ComponentIdView<IdType>& other) const { return raw_id_ != other.raw(); }
  bool operator==(const IdType& other) const { return raw_id_ == other.raw(); }
  bool operator!=(const IdType& other) const { return raw_id_ != other.raw(); }

  inline size_t raw() const { return raw_id_; }
  inline int raw_as_int() const { return static_cast<int>(raw_id_); }

  inline bool expired() const { return !resin::ComponentIdRegistry<IdType>::instance().is_registered(raw_id_); }

 private:
  size_t raw_id_;
};

}  // namespace resin

#endif
