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

template <typename Obj>
class IdRegistry {
 public:
  static const size_t kMaxObjects = 10000;

  static IdRegistry<Obj>& instance() {
    // thread-safe according to
    // https://stackoverflow.com/questions/1661529/is-meyers-implementation-of-the-singleton-pattern-thread-safe
    static IdRegistry<Obj> instance;
    return instance;
  }

  size_t register_id() {
    mutex_.lock();

    if (freed_.empty()) {
      log_throw(ObjectsOverflowException());
    }

    size_t new_id = freed_.top();
    freed_.pop();
    is_registered_[new_id] = true;

    return new_id;
  }

  bool is_registered(size_t id) {
    mutex_.lock();

    if (id >= kMaxObjects) {
      return false;
    }

    return is_registered_[id];
  }

  void unregister_id(size_t id) {
    mutex_.lock();

    if (id >= kMaxObjects) {
      Logger::warn("Detected an attempt to unregister a non-existent id [{}].", id);
      return;
    }

    is_registered_[id] = false;
    freed_.push(id);
  }

  IdRegistry(const IdRegistry&)            = delete;
  IdRegistry& operator=(const IdRegistry&) = delete;

 private:
  IdRegistry() {
    for (size_t i = kMaxObjects; i-- > 0;) {
      freed_.push(i);
    }
    std::fill(is_registered_.begin(), is_registered_.end(), false);
  }

  std::stack<size_t> freed_;
  std::array<bool, kMaxObjects> is_registered_{};
  std::mutex mutex_;
};

template <typename Obj>
struct IdView;

template <typename Obj>
struct Id {
 public:
  Id() : raw_id_(resin::IdRegistry<Obj>::instance().register_id()) {}
  ~Id() { resin::IdRegistry<Obj>::instance().unregister_id(raw_id_); }

  Id(const Id<Obj>& other)                 = delete;
  Id<Obj>& operator=(const Id<Obj>& other) = delete;

  bool operator==(const Id<Obj>& other) const { return raw_id_ == other.raw_id_; }
  bool operator!=(const Id<Obj>& other) const { return raw_id_ != other.raw_id_; }

  bool operator==(const IdView<Obj>& other) const { return raw_id_ == other.raw_id_; }
  bool operator!=(const IdView<Obj>& other) const { return raw_id_ != other.raw_id_; }

  inline size_t raw() const { return raw_id_; }

  inline IdView<Obj> view() const { return IdView(*this); }

 private:
  size_t raw_id_;
};

template <typename Obj>
struct IdView {
  IdView() = delete;

  explicit IdView(const Id<Obj> id) : raw_id_(id.raw_id_) {}

  bool operator==(const IdView<Obj>& other) const { return raw_id_ == other.raw_id_; }
  bool operator!=(const IdView<Obj>& other) const { return raw_id_ != other.raw_id_; }

  bool operator==(const Id<Obj>& other) const { return raw_id_ == other.raw_id_; }
  bool operator!=(const Id<Obj>& other) const { return raw_id_ != other.raw_id_; }

  inline size_t raw() const { return raw_id_; }

  inline bool expired() const { return resin::IdRegistry<Obj>::instance().is_registered(raw_id_); }

 private:
  size_t raw_id_;
};

}  // namespace resin

#endif
