#include <gtest/gtest.h>

#include <libresin/core/camera.hpp>

namespace resin {

TEST(PerspectiveCameraTest, UpdateDimensions) {
  // given
  const float fov            = 45.0f;
  const float aspectRatio    = 16.0f / 9.0f;
  const float nearPlane      = 0.1f;
  const float farPlane       = 100.0f;
  const float expectedHeight = 0.082842715;
  const float expectedWidth  = 0.14727594;

  // when
  const resin::PerspectiveCamera camera(fov, aspectRatio, nearPlane, farPlane);

  // then
  EXPECT_NEAR(camera.height, expectedHeight, 1e-5);
  EXPECT_NEAR(camera.width, expectedWidth, 1e-5);
}

}  // namespace resin