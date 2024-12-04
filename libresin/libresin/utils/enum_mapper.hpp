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

template <EnumWithCountConcept EnumType>
struct EnumMapping {
  using enum_type = std::remove_cv_t<std::remove_reference_t<EnumType>>;

  constexpr EnumMapping() = default;

  template <size_t N>
  constexpr explicit EnumMapping(const std::string_view (&input_names)[N]) {
    static_assert(N == static_cast<size_t>(EnumType::_Count), "Names count should be the same as enum entries count.");
    for (size_t i = 0; i < N; ++i) {
      names_[i] = input_names[i];
    }
    size = N;
  }

  constexpr std::string_view get_name(enum_type enum_entry) const {
    auto index = static_cast<size_t>(enum_entry);
    return (index < size) ? names_[index] : std::string_view{};
  }

  std::array<std::string_view, static_cast<size_t>(EnumType::_Count)> names_ = {};
  size_t size                                                                = 0;
};

}  // namespace resin

#endif
