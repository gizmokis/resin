#pragma once

#include <cstddef>
#include <utility>

namespace resin {

/**
 * @brief Wraps std::array<T, N> and allows for vector-like operations. This vector does not grow, the
 * reserved memory (capacity) is always equal to N.
 *
 * @tparam T
 * @tparam N
 */
template <typename T, std::size_t N>
class StaticVector {
 public:
  StaticVector()                                   = default;
  StaticVector(StaticVector&&) noexcept            = default;
  StaticVector(const StaticVector&)                = default;
  StaticVector& operator=(StaticVector&&) noexcept = default;
  StaticVector& operator=(const StaticVector&)     = default;

  size_t size() const { return size_; }
  size_t capacity() const { return data_.size(); }

  void emplace(T&& val) { data_[size_++] = std::move(val); }
  void pop() { --size_; }

  auto begin() { return data_.begin(); }
  auto end() { return data_.begin() + size_; }

  auto begin() const { return data_.begin(); }
  auto end() const { return data_.begin() + size_; }

  T& operator[](size_t ind) { return data_[ind]; }
  const T& operator[](size_t ind) const { return data_[ind]; }

 private:
  std::array<T, N> data_;
  size_t size_ = 0;
};

}  // namespace resin
