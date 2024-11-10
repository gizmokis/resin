#include <gtest/gtest.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_relational.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/string_cast.hpp>

// Helper function to compare two glm vectors with a tolerance.
template <glm::length_t L, typename T, glm::qualifier Q>
::testing::AssertionResult AreGLMVectorsNear(const char* expected_expr, const char* actual_expr,
                                             const char* epsilon_expr, const glm::vec<L, T, Q>& expected,
                                             const glm::vec<L, T, Q>& actual, T epsilon) {
  if (glm::any(glm::epsilonNotEqual(expected, actual, epsilon))) {
    return ::testing::AssertionFailure() << "Expected equality with tolerance " << epsilon_expr
                                         << " of these vectors:\n\t" << expected_expr
                                         << "\n\t\tWhich is: " << glm::to_string(expected) << "\n\t" << actual_expr
                                         << "\n\t\tWhich is: " << glm::to_string(actual) << "\n";
  }

  return ::testing::AssertionSuccess();
}

// Helper function to compare two glm matrices with a tolerance.
template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
::testing::AssertionResult AreGLMMatricesNear(const char* expected_expr, const char* actual_expr,
                                              const char* epsilon_expr, const glm::mat<C, R, T, Q>& expected,
                                              const glm::mat<C, R, T, Q>& actual, T epsilon) {
  if (glm::any(glm::notEqual(expected, actual, epsilon))) {
    return ::testing::AssertionFailure() << "Expected equality with tolerance " << epsilon_expr
                                         << " of these matrices:\n\t" << expected_expr
                                         << "\n\t\tWhich is: " << glm::to_string(expected) << "\n\t" << actual_expr
                                         << "\n\t\tWhich is: " << glm::to_string(actual) << "\n";
  }

  return ::testing::AssertionSuccess();
}

// Helper function to compare two glm quaternions with a tolerance.
template <typename T, glm::qualifier Q>
::testing::AssertionResult AreGLMQuaternionRotationsNear(const char* expected_expr, const char* actual_expr,
                                                         const char* epsilon_expr, const glm::qua<T, Q>& expected,
                                                         const glm::qua<T, Q>& actual, T epsilon) {
  if (!glm::epsilonEqual(glm::abs(glm::dot(expected, actual)), static_cast<T>(1), epsilon)) {
    return ::testing::AssertionFailure() << "Expected equality with tolerance " << epsilon_expr
                                         << " of these rotations:\n\t" << expected_expr
                                         << "\n\t\tWhich is: " << glm::to_string(expected) << "\n\t" << actual_expr
                                         << "\n\t\tWhich is: " << glm::to_string(actual) << "\n"
                                         << "NOTE: two quaternions represent the same rotation iff |q1.q2|=1\n";
  }

  return ::testing::AssertionSuccess();
}

// Macro for glm vector comparison.
#define EXPECT_GLM_VEC_NEAR(expected, actual, epsilon) EXPECT_PRED_FORMAT3(AreGLMVectorsNear, expected, actual, epsilon)

// Macro for glm matrix comparison.
#define EXPECT_GLM_MAT_NEAR(expected, actual, epsilon) \
  EXPECT_PRED_FORMAT3(AreGLMMatricesNear, expected, actual, epsilon)

// Macro for glm quaternion comparison.
#define EXPECT_GLM_ROT_NEAR(expected, actual, epsilon) \
  EXPECT_PRED_FORMAT3(AreGLMQuaternionRotationsNear, expected, actual, epsilon)