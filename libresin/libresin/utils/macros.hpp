#ifndef RESIN_MACROS_HPP
#define RESIN_MACROS_HPP

#define DISABLE_COPY_AND_MOVE(ClassName)           \
  ClassName(const ClassName&)            = delete; \
  ClassName(ClassName&&)                 = delete; \
  ClassName& operator=(const ClassName&) = delete; \
  ClassName& operator=(ClassName&&)      = delete;

// NOLINTBEGIN
#define RESIN_VALIDATE_VARIANT_TYPES(TVariant, CVariant)                     \
  template <typename Variant>                                                \
  struct ValidateVariant_##TVariant;                                         \
                                                                             \
  template <typename... Types>                                               \
  struct ValidateVariant_##TVariant<std::variant<Types...>> {                \
    static constexpr bool kAreVariantTypesValid = (CVariant<Types> && ...);  \
  };                                                                         \
                                                                             \
  static_assert(ValidateVariant_##TVariant<TVariant>::kAreVariantTypesValid, \
                "Not all variant types satisfy the required constraint")
// NOLINTEND
#endif
