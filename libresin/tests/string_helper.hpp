#pragma once
#include <gtest/gtest.h>

#include <algorithm>
#include <ranges>
#include <string>

inline ::testing::AssertionResult AreStringsEqualIgnoringWhitespace(const char* /*expected_expr*/,
                                                                    const char* /*actual_expr*/,
                                                                    std::string_view expected,
                                                                    std::string_view actual) {
  static constexpr auto kFilter = std::views::filter([](char c) { return c != '\n' && c != '\r' && c != ' '; });

  auto expected_filtered = expected | kFilter;
  auto actual_filtered   = actual | kFilter;

  if (std::ranges::equal(expected_filtered, actual_filtered)) {
    return ::testing::AssertionSuccess();
  }

  std::string expected_str(expected_filtered.begin(), expected_filtered.end());
  std::string actual_str(actual_filtered.begin(), actual_filtered.end());

  return ::testing::AssertionFailure() << "Expected equality of strings ignoring whitespace:\n"
                                       << "  Expected (filtered): \"" << expected_str << "\"\n"
                                       << "  Actual   (filtered): \"" << actual_str << "\"\n"
                                       << "  Original expected: \"" << expected << "\"\n"
                                       << "  Original actual:   \"" << actual << "\"";
}

inline ::testing::AssertionResult StringContainsIgnoringWhitespace(const char* /*str_expr*/,
                                                                   const char* /*substr_expr*/, std::string_view str,
                                                                   std::string_view substr) {
  static constexpr auto kFilter = std::views::filter([](char c) { return c != '\n' && c != '\r' && c != ' '; });

  auto str_filtered_r    = str | kFilter;
  auto substr_filtered_r = substr | kFilter;

  std::string str_filtered(str_filtered_r.begin(), str_filtered_r.end());
  std::string substr_filtered(substr_filtered_r.begin(), substr_filtered_r.end());

  if (str_filtered.contains(substr_filtered)) {
    return ::testing::AssertionSuccess();
  }

  return ::testing::AssertionFailure() << "Expected: " << str << " contains " << substr << " (ignoring whitespace)\n"
                                       << "  Filtered string:    \"" << str_filtered << "\"\n"
                                       << "  Filtered substring: \"" << substr_filtered << "\"";
}

// NOLINTBEGIN

#define EXPECT_STRINGS_EQ_IGNORING_WHITESPACE(expected, actual) \
  EXPECT_PRED_FORMAT2(AreStringsEqualIgnoringWhitespace, expected, actual)

#define EXPECT_STRING_CONTAINS_IGNORING_WHITESPACE(str, substr) \
  EXPECT_PRED_FORMAT2(StringContainsIgnoringWhitespace, str, substr)

// NOLINTEND
