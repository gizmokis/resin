#include <gtest/gtest.h>

#include <libresin/core/transform.hpp>
#include <print>
#include <tests/glm_helper.hpp>

constexpr float kPi = glm::pi<float>();

class TransformTest : public testing::Test {
 protected:
  TransformTest() : parent_transform_(glm::vec3(1, 2, 3), glm::quat(glm::vec3(0, kPi / 2, 0)), 2) {
    transform_.set_parent(parent_transform_);
  }

  // at:       1,2,3
  // rotation: 90deg around Y axis
  // scale:    2
  resin::Transform parent_transform_;

  // at:       0,0,0
  // rotation: none
  // scale:    1
  resin::Transform transform_;
};

/**
 * Defaults
 */

TEST_F(TransformTest, DefaultConstructorWorks) {
  // given / when
  const resin::Transform transform;

  // then
  EXPECT_EQ(transform.local_pos(), glm::vec3(0, 0, 0));
  EXPECT_EQ(transform.local_rot(), glm::quat(1, 0, 0, 0));
  EXPECT_EQ(transform.local_scale(), 1);
}

TEST_F(TransformTest, ParametricConstructorWorks) {
  // given
  const glm::vec3 pos(1, 2, 3);
  const glm::quat rot(4, 5, 6, 7);
  const float scale = 8;

  // when
  const resin::Transform transform(pos, rot, scale);

  // then
  EXPECT_EQ(transform.local_pos(), pos);
  EXPECT_EQ(transform.local_rot(), rot);
  EXPECT_EQ(transform.local_scale(), scale);
}

TEST_F(TransformTest, DefaultMatricesAreIdentity) {
  // given / when
  const resin::Transform transform;

  // then
  EXPECT_EQ(transform.local_to_parent_matrix(), glm::mat4(1));
  EXPECT_EQ(transform.local_to_world_matrix(), glm::mat4(1));
  EXPECT_EQ(transform.parent_to_local_matrix(), glm::mat4(1));
  EXPECT_EQ(transform.world_to_local_matrix(), glm::mat4(1));
}

/**
 * Transformations
 */
TEST_F(TransformTest, MatricesAreCalculatedProperly) {
  // given
  // calculated on paper
  const glm::mat4 parent_expected(glm::vec4(0, 0, -1, 0), glm::vec4(0, 2, 0, 0), glm::vec4(3, 0, 0, 0),
                                  glm::vec4(1, 2, 3, 1));
  const glm::mat4 expected(glm::vec4(0, 0, -1, 0), glm::vec4(3, 0, 0, 0), glm::vec4(0, -2, 0, 0),
                           glm::vec4(1, 2, 3, 1));

  // when
  transform_.set_local_rot(glm::vec3(kPi / 2, 0, 0));

  // then
  EXPECT_GLM_MAT_NEAR(parent_expected, parent_transform_.local_to_parent_matrix(), 1e-5F);
  EXPECT_GLM_MAT_NEAR(expected, transform_.local_to_world_matrix(), 1e-5F);
}

TEST_F(TransformTest, InversesAreCalculatedProperly) {
  // given
  glm::mat4 identity(1.0F);

  // when / then
  EXPECT_GLM_MAT_NEAR(identity, parent_transform_.local_to_parent_matrix() * parent_transform_.parent_to_local_matrix(),
                      1e-5F);
  EXPECT_GLM_MAT_NEAR(identity, parent_transform_.local_to_world_matrix() * parent_transform_.world_to_local_matrix(),
                      1e-5F);
  EXPECT_GLM_MAT_NEAR(identity, transform_.local_to_parent_matrix() * transform_.parent_to_local_matrix(), 1e-5F);
  EXPECT_GLM_MAT_NEAR(identity, transform_.local_to_world_matrix() * transform_.world_to_local_matrix(), 1e-5F);
}

TEST_F(TransformTest, LocalOrientationIsCalculatedProperly) {
  // given
  transform_.set_local_rot(glm::vec3(kPi / 2, 0, 0));
  glm::vec3 front = transform_.local_front();
  glm::vec3 right = transform_.local_right();
  glm::vec3 up    = transform_.local_up();

  // when
  glm::mat3 orientation = transform_.local_orientation();

  // then
  EXPECT_GLM_VEC_NEAR(right, orientation[0], 1e-5F);
  EXPECT_GLM_VEC_NEAR(up, orientation[1], 1e-5F);
  EXPECT_GLM_VEC_NEAR(front, orientation[2], 1e-5F);
}

TEST_F(TransformTest, OrientationIsCalculatedProperly) {
  // given
  transform_.set_local_rot(glm::vec3(kPi / 2, 0, 0));
  glm::vec3 front = transform_.front();
  glm::vec3 right = transform_.right();
  glm::vec3 up    = transform_.up();

  // when
  glm::mat3 orientation = transform_.orientation();

  // then
  EXPECT_GLM_VEC_NEAR(right, orientation[0], 1e-5F);
  EXPECT_GLM_VEC_NEAR(up, orientation[1], 1e-5F);
  EXPECT_GLM_VEC_NEAR(front, orientation[2], 1e-5F);
}

TEST_F(TransformTest, PosIsTransformedProperly) {
  // given
  const glm::vec3 position(1, 1, 1);
  // initial:    (1, 1, 1)
  // scaled:     (2, 2, 2)
  // rotated:    (2, 2,-2)
  // translated: (2, 4, 1)
  glm::vec3 expected(2, 4, 1);

  // when
  transform_.set_local_pos(position);

  // then
  EXPECT_GLM_VEC_NEAR(expected, transform_.pos(), 1e-5F);
}

TEST_F(TransformTest, RotIsTransformedProperly) {
  // given
  // rotation of 90deg around X axis
  const glm::quat rot(glm::vec3(kPi / 2, 0, 0));
  // rotation of 120deg around axis of (1,1,-1)
  glm::vec3 axis     = glm::normalize(glm::vec3(1, 1, -1));
  glm::quat expected = glm::angleAxis(2 * kPi / 3, axis);

  // when
  transform_.set_local_rot(rot);

  // then
  EXPECT_GLM_ROT_NEAR(expected, transform_.rot(), 1e-5F);
}
