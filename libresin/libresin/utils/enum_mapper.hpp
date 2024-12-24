#ifndef RESIN_ENUM_MAPPER_HPP
#define RESIN_ENUM_MAPPER_HPP

#include <array>
#include <cstddef>
#include <string_view>
#include <type_traits>

namespace resin {

template <typename T>
concept EnumWithCountConcept = requires {
  std::is_enum_v<std::remove_cv_t<std::remove_reference_t<T>>>;
  { T::_Count } -> std::convertible_to<T>;
};

template <typename T>
concept EnumMappingValueConcept = requires { std::is_move_assignable<T>(); };

template <EnumWithCountConcept EnumType, EnumMappingValueConcept Value>
struct EnumMapping;

template <EnumWithCountConcept EnumType, typename Value>
class EnumMappingIterator {
 public:
  using enum_type = typename EnumMapping<EnumType, Value>::enum_type;

  EnumMappingIterator(const EnumMapping<EnumType, Value>& mapping, size_t index) : mapping_(mapping), index_(index) {}

  std::pair<enum_type, Value> operator*() const {
    return {static_cast<enum_type>(index_), mapping_.get_value(static_cast<enum_type>(index_))};
  }

  EnumMappingIterator& operator++() {
    ++index_;
    return *this;
  }

  EnumMappingIterator operator++(int) {
    EnumMappingIterator temp = *this;
    ++(*this);
    return temp;
  }

  bool operator==(const EnumMappingIterator& other) const { return index_ == other.index_; }
  bool operator!=(const EnumMappingIterator& other) const { return !(*this == other); }

 private:
  const EnumMapping<EnumType, Value>& mapping_;
  size_t index_;
};

template <EnumWithCountConcept EnumType, EnumMappingValueConcept Value>
struct EnumMapping {
  using enum_type = std::remove_cv_t<std::remove_reference_t<EnumType>>;

  constexpr EnumMapping() = default;

  template <size_t N>
  constexpr explicit EnumMapping(const Value (&input_names)[N]) {
    static_assert(N == static_cast<size_t>(EnumType::_Count), "Names count should be the same as enum entries count.");
    for (size_t i = 0; i < N; ++i) {
      names_[i] = std::move(input_names[i]);
    }
    size = N;
  }

  auto begin() const { return EnumMappingIterator<EnumType, Value>(*this, 0); }

  auto end() const { return EnumMappingIterator<EnumType, Value>(*this, size); }

  constexpr Value get_value(enum_type enum_entry) const { return names_[static_cast<size_t>(enum_entry)]; }

  std::array<Value, static_cast<size_t>(EnumType::_Count)> names_ = {};
  size_t size                                                     = 0;
};

template <EnumWithCountConcept EnumType>
using StringEnumMapping = EnumMapping<EnumType, std::string_view>;

}  // namespace resin

#endif
