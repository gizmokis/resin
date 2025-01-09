#ifndef RESIN_ENUM_MAPPER_HPP
#define RESIN_ENUM_MAPPER_HPP

#include <array>
#include <cstddef>
#include <optional>
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
class EnumMapper;

template <EnumWithCountConcept EnumType, typename Value>
class EnumMappingIterator {
 public:
  using enum_type = typename EnumMapper<EnumType, Value>::enum_type;

  EnumMappingIterator(const EnumMapper<EnumType, Value>& mapping, size_t index) : mapping_(mapping), index_(index) {}

  std::pair<enum_type, Value> operator*() const {
    return {static_cast<enum_type>(index_), mapping_[static_cast<enum_type>(index_)]};
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
  const EnumMapper<EnumType, Value>& mapping_;
  size_t index_;
};

template <EnumWithCountConcept EnumType, EnumMappingValueConcept Value>
class EnumMapper {
 public:
  using enum_type                   = std::remove_cv_t<std::remove_reference_t<EnumType>>;
  static constexpr size_t kEnumSize = static_cast<size_t>(EnumType::_Count);

  EnumMapper() = delete;

  template <size_t N>
  consteval explicit EnumMapper(const std::pair<EnumType, Value> (&values_map)[N]) {
    static_assert(N == kEnumSize, "Names count should be the same as enum entries count.");

    for (size_t i = 0; i < kEnumSize; ++i) {
      present_[i] = false;
    }

    for (size_t i = 0; i < kEnumSize; ++i) {
      auto idx      = static_cast<size_t>(values_map[i].first);
      names_[idx]   = values_map[i].second;
      present_[idx] = true;
    }

    validate_mapping(std::make_index_sequence<kEnumSize>{});
  }

  constexpr auto begin() const { return EnumMappingIterator<EnumType, Value>(*this, 0); }
  constexpr auto end() const { return EnumMappingIterator<EnumType, Value>(*this, kEnumSize); }

  constexpr Value value(enum_type enum_entry) const { return names_[static_cast<size_t>(enum_entry)]; }
  constexpr Value operator[](enum_type enum_entry) const { return names_[static_cast<size_t>(enum_entry)]; }

  // Keep in mind this mapping may be neither injective nor surjective
  constexpr std::optional<enum_type> from_value(Value value) const {
    for (size_t i = 0; i < kEnumSize; ++i) {
      if (names_[i] == value) {
        return std::make_optional(static_cast<enum_type>(i));
      }
    }

    return std::nullopt;
  }

 private:
  template <size_t... Is>
  consteval void validate_mapping(std::index_sequence<Is...>) const {
    ((check_present(Is)), ...);
  }

  consteval void check_present(size_t idx) const {
    if (!present_[idx]) {
      throw "Missing mapping for enum value";
    }
  }

  std::array<Value, kEnumSize> names_  = {};
  std::array<bool, kEnumSize> present_ = {};
};

template <EnumWithCountConcept EnumType>
using StringEnumMapper = EnumMapper<EnumType, std::string_view>;

}  // namespace resin

#endif
