#include <gtest/gtest.h>

#include <libresin/core/camera.hpp>

namespace resin {

TEST(CameraTest, UpdateDimensions) {
  // given
  const float fov             = 45.0f;
  const float aspect_ratio    = 16.0f / 9.0f;
  const float near_plane      = 0.1f;
  const float far_plane       = 100.0f;
  const float expected_height = 0.082842715;
  const float expected_width  = 0.14727594;

  // when
  const resin::Camera camera(false, fov, aspect_ratio, near_plane, far_plane);

  // then
  EXPECT_NEAR(camera.height(), expected_height, 1e-5);
  EXPECT_NEAR(camera.width(), expected_width, 1e-5);
}

}  // namespace resin