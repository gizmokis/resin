#include <gtest/gtest.h>
#include <libresin/core/transform.hpp>

// Demonstrate some basic assertions.
TEST(TransformTest, Main) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);

  resin::Transform l;
}