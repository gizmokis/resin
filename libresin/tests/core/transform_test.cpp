#include <gtest/gtest.h>

#include <libresin/core/transform.hpp>

class TransformTest : public testing::Test {
 protected:
  resin::Transform transform_;
  resin::Transform parent_transform_;
};

TEST_F(TransformTest, DefaultConstructor) {
  // given / when / then
  EXPECT_EQ(transform_.local_pos(), glm::vec3(0, 0, 0));
  EXPECT_EQ(transform_.local_rot(), glm::quat(1, 0, 0, 0));
  EXPECT_EQ(transform_.local_scale(), glm::vec3(1, 1, 1));
}

TEST_F(TransformTest, ParametricConstructor) {
  // given
  const glm::vec3 pos(1, 2, 3);
  const glm::quat rot(4, 5, 6, 7);
  const glm::vec3 scale(8, 9, 10);

  // when
  const resin::Transform transform(pos, rot, scale);

  // then
  EXPECT_EQ(transform.local_pos(), pos);
  EXPECT_EQ(transform.local_rot(), rot);
  EXPECT_EQ(transform.local_scale(), scale);
}

TEST_F(TransformTest, DefaultMatrices) {
  // given / when / then
  EXPECT_EQ(transform_.local_to_world_matrix(), glm::mat4(1));
  EXPECT_EQ(transform_.world_to_local_matrix(), glm::mat4(1));
}

TEST_F(TransformTest, PositionGetSet) {
  // given
  const glm::vec3 expected(1, 2, 3);

  // when
  transform_.set_local_pos(expected);

  // then
  EXPECT_EQ(transform_.local_pos(), expected);
}

TEST_F(TransformTest, RotationGetSet) {
  // given
  const glm::vec3 expected(1, 2, 3);

  // when
  transform_.set_local_pos(expected);

  // then
  EXPECT_EQ(transform_.local_pos(), expected);
}