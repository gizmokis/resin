#include <gtest/gtest.h>

#include <filesystem>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/sdf_primitive_type_manager.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <tests/libresin/test_consts.hpp>

class PrimitiveTypeManagerTest : public testing::Test {
 protected:
  const std::filesystem::path data_path_      = RESIN_TESTS_DATA_PATH;
  const std::filesystem::path resources_path_ = data_path_ / "core" / "resources";
};

TEST_F(PrimitiveTypeManagerTest, ShaderTypeIsNotCreatedTwice) {
  // given
  resin::ShaderResourceManager sh_resman;
  resin::SDFPrimitiveTypeManager manager;

  // when
  auto id1 = manager.add_type_from_shader_res(sh_resman.get_res_ptr(resources_path_ / "sdf_func" / "sphere.sdf"));
  auto id2 = manager.add_type_from_shader_res(sh_resman.get_res_ptr(resources_path_ / "sdf_func" / "sphere.sdf"));

  // then
  EXPECT_EQ(id1, id2);
}

TEST_F(PrimitiveTypeManagerTest, PrimitiveNodeHasEmptyStateWhenTypeIsDeleted) {
  // given
  resin::ShaderResourceManager sh_resman;
  resin::SDFTree tree;
  auto id = tree.primitive_type_manager().add_type_from_shader_res(
      sh_resman.get_res_ptr(resources_path_ / "sdf_func" / "sphere.sdf"));

  // when
  tree.root().push_back_child<resin::PrimitiveNode>(resin::SDFBinaryOperation::Union, id);
  tree.primitive_type_manager().clear();

  // then
  auto it    = tree.root().begin();
  auto& prim = tree.primitive(*it);
  EXPECT_EQ(prim.type(), std::nullopt);
}

TEST_F(PrimitiveTypeManagerTest, PrimitiveTypeManagerCorrectlyGeneratesSDFsCode) {
  // given
  resin::ShaderResourceManager sh_resman;
  resin::SDFTree tree;
  auto sphere_id = tree.primitive_type_manager().add_type_from_shader_res(
      sh_resman.get_res_ptr(resources_path_ / "sdf_func" / "sphere.sdf"));
  auto cube_id = tree.primitive_type_manager().add_type_from_shader_res(
      sh_resman.get_res_ptr(resources_path_ / "sdf_func" / "cube.sdf"));
  tree.root().push_back_child<resin::PrimitiveNode>(resin::SDFBinaryOperation::Union, sphere_id);
  tree.root().push_back_child<resin::PrimitiveNode>(resin::SDFBinaryOperation::Union, cube_id);

  // when
  auto sdfs_glsl = tree.primitive_type_manager().sdfs_glsl();

  // then
  EXPECT_TRUE(sdfs_glsl.contains("Sphere"));
  EXPECT_TRUE(sdfs_glsl.contains("Cube"));
  EXPECT_TRUE(sdfs_glsl.contains("SDF"));
  EXPECT_TRUE(sdfs_glsl.contains("radius"));
  EXPECT_TRUE(sdfs_glsl.contains("width"));
  EXPECT_TRUE(sdfs_glsl.contains("height"));
  EXPECT_TRUE(sdfs_glsl.contains("depth"));
  EXPECT_TRUE(sdfs_glsl.contains("length(max(d,0.0));"));
  EXPECT_TRUE(sdfs_glsl.contains("length(pos)"));
  EXPECT_TRUE(sdfs_glsl.contains("return"));
  EXPECT_FALSE(sdfs_glsl.contains("#name"));
  EXPECT_FALSE(sdfs_glsl.contains("sdf"));
}

TEST_F(PrimitiveTypeManagerTest, PrimitiveTypeIsNotOverridenByADifferentTypeWithTheSameName) {
  // given
  resin::ShaderResourceManager sh_resman;
  resin::SDFTree tree;
  auto cube_id = tree.primitive_type_manager().add_type_from_shader_res(
      sh_resman.get_res_ptr(resources_path_ / "sdf_func" / "cube.sdf"));
  auto cube2_id = tree.primitive_type_manager().add_type_from_shader_res(
      sh_resman.get_res_ptr(resources_path_ / "sdf_func" / "cube2.sdf"));

  // when
  tree.root().push_back_child<resin::PrimitiveNode>(resin::SDFBinaryOperation::Union, cube_id);
  tree.root().push_back_child<resin::PrimitiveNode>(resin::SDFBinaryOperation::Union, cube2_id);

  // then
  EXPECT_NE(cube_id, cube2_id);
  EXPECT_NE(tree.primitive_type_manager().type_by_id(cube_id).name,
            tree.primitive_type_manager().type_by_id(cube2_id).name);
}
