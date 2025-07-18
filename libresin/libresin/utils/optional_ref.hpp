#pragma once

#include <optional>
#include <type_traits>

namespace resin {

// NOLINTBEGIN
template <typename T>
class optional_ref {
 public:
  optional_ref() noexcept : ptr(nullptr) {}
  optional_ref(std::nullopt_t) noexcept : ptr(nullptr) {}
  optional_ref(T& ref) noexcept : ptr(&ref) {}
  optional_ref(const optional_ref& other) noexcept            = default;
  optional_ref& operator=(const optional_ref& other) noexcept = default;

  template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
  optional_ref(const optional_ref<U>& other) noexcept : ptr(other.ptr) {}

  bool has_value() const noexcept { return ptr != nullptr; }
  explicit operator bool() const noexcept { return has_value(); }

  T& value() const {
    if (!ptr) throw std::bad_optional_access();
    return *ptr;
  }

  T& operator*() const noexcept { return *ptr; }

  T* operator->() const noexcept { return ptr; }

  template <typename U>
    requires std::is_convertible_v<U, T>
  T& value_or(U&& fallback) const {
    return ptr ? *ptr : static_cast<T&>(fallback);
  }

  bool operator==(std::nullopt_t) const noexcept { return !has_value(); }
  bool operator!=(std::nullopt_t) const noexcept { return has_value(); }

  void reset() noexcept { ptr = nullptr; }

  void swap(optional_ref& other) noexcept { std::swap(ptr, other.ptr); }

 private:
  T* ptr;

  template <typename U>
  friend class optional_ref;
};

template <typename T>
bool operator==(const optional_ref<T>& lhs, const optional_ref<T>& rhs) {
  return lhs.has_value() == rhs.has_value() && (!lhs.has_value() || *lhs == *rhs);
}

template <typename T>
bool operator!=(const optional_ref<T>& lhs, const optional_ref<T>& rhs) {
  return !(lhs == rhs);
}

template <typename T>
bool operator<(const optional_ref<T>& lhs, const optional_ref<T>& rhs) {
  return rhs.has_value() && (!lhs.has_value() || *lhs < *rhs);
}

template <typename T>
void swap(optional_ref<T>& lhs, optional_ref<T>& rhs) noexcept {
  lhs.swap(rhs);
}

// NOLINTEND

}  // namespace resin
