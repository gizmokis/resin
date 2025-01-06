#include <gtest/gtest.h>

#include <algorithm>
#include <libresin/core/material.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/transform.hpp>
#include <optional>
#include <print>
#include <tests/glm_helper.hpp>

class SDFTreeTest : public testing::Test {};

TEST_F(SDFTreeTest, SDFShaderIsCorrectlyGenerated) {
  // given
  //      +
  // +          -
  //       +    ^    -
  //           + -
  resin::SDFTree tree;
  tree.root().push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union);
  auto group1 = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Diff).node_id();
  tree.group(group1).push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union);
  auto group2 = tree.group(group1).push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Inter).node_id();
  tree.group(group1).push_back_child<resin::SphereNode>(resin::SDFBinaryOperation::Diff);
  tree.group(group2).push_back_child<resin::SphereNode>(resin::SDFBinaryOperation::Union);
  tree.group(group2).push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Diff);

  // when
  auto sh_code_single_prim_arr = tree.gen_shader_code(resin::GenShaderMode::SinglePrimitiveArray);

  // then
  ASSERT_EQ(
      "opDiff(sdCube(pos,0),opDiff(opInter(sdCube(pos,1),opDiff(sdSphere(pos,3),sdCube(pos,4),0.5),0.5),sdSphere(pos,2),0.5),0.5)",
      sh_code_single_prim_arr);
}

TEST_F(SDFTreeTest, SDFShaderGenerationOmitsShallowNodes) {
  // given
  //      +
  // +          -
  //       +    ^    -
  //           + -
  resin::SDFTree tree;
  tree.root().push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union);
  auto& group1 = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Diff);
  group1.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union);
  auto& group2 = group1.push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Inter);
  group1.push_back_child<resin::SphereNode>(resin::SDFBinaryOperation::Diff);
  group2.push_back_child<resin::SphereNode>(resin::SDFBinaryOperation::Union);
  auto& group3 = group2.push_front_child<resin::GroupNode>(resin::SDFBinaryOperation::Union)
                     .push_front_child<resin::GroupNode>(resin::SDFBinaryOperation::Union)
                     .push_front_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  group3.push_front_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  group3.push_front_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  group3.push_front_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  group2.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Diff);
  auto& group4 = group1.push_front_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  group4.push_front_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  group4.push_front_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  group4.push_front_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  group4.push_front_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);

  // when
  auto sh_code_single_prim_arr = tree.gen_shader_code(resin::GenShaderMode::SinglePrimitiveArray);

  // then
  ASSERT_EQ(
      "opDiff(sdCube(pos,0),opDiff(opInter(sdCube(pos,1),opDiff(sdSphere(pos,3),sdCube(pos,4),0.5),0.5),sdSphere(pos,2),0.5),0.5)",
      sh_code_single_prim_arr);
}

TEST_F(SDFTreeTest, NodesAreCorrectlyMoved) {
  // given
  //       o
  //  o    o    o
  //      o o  o o
  resin::SDFTree tree;
  auto group1 = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Union).node_id();
  tree.root().push_front_child<resin::SphereNode>(resin::SDFBinaryOperation::Union);
  tree.group(group1).push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union);
  auto group1_child = tree.group(group1).push_back_child<resin::SphereNode>(resin::SDFBinaryOperation::Union).node_id();
  auto group2       = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Inter).node_id();
  tree.group(group2).push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Xor);
  tree.group(group2).push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Inter);

  // when
  auto child_ptr = tree.root().detach_child(group2);
  tree.group(group1).insert_before_child(group1_child, std::move(child_ptr));

  // then
  //      o
  //  o       o
  //      o   o   o
  //         o o
  auto it = tree.root().begin();
  ASSERT_TRUE(tree.node(*it).is_leaf());
  ++it;
  ASSERT_TRUE(!tree.node(*it).is_leaf());

  it = tree.group(*it).begin();
  ASSERT_TRUE(tree.node(*it).is_leaf());
  ++it;
  ASSERT_TRUE(!tree.node(*it).is_leaf());
  auto it_cpy = it;
  ++it;
  ASSERT_TRUE(tree.node(*it).is_leaf());

  it = tree.group(*it_cpy).begin();
  ASSERT_TRUE(tree.node(*it).is_leaf());
  ++it;
  ASSERT_TRUE(tree.node(*it).is_leaf());
}

TEST_F(SDFTreeTest, NodesAreCorrectlyCopied) {
  // given
  //       o
  //  o    o    o
  //      o o  o o
  resin::SDFTree tree;
  auto group1 = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Union).node_id();
  tree.root().push_front_child<resin::SphereNode>(resin::SDFBinaryOperation::Union);
  auto group1_child = tree.group(group1).push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union).node_id();
  tree.group(group1).push_back_child<resin::SphereNode>(resin::SDFBinaryOperation::Union);
  auto group2 = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Inter).node_id();
  tree.group(group2).push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Xor);
  tree.group(group2).push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Inter);

  // when
  auto child_ptr = tree.node(group2).copy();
  tree.group(group1).insert_after_child(group1_child, std::move(child_ptr));

  // then
  //          o
  //  o       o        o
  //      o   o   o   o o
  //         o o
  auto it = tree.root().begin();
  ASSERT_TRUE(tree.node(*it).is_leaf());
  ++it;
  auto subtree1 = tree.group(*it).begin();
  ASSERT_TRUE(!tree.node(*it).is_leaf());
  ++it;
  auto subtree2 = tree.group(*it).begin();
  ASSERT_TRUE(!tree.node(*it).is_leaf());

  ASSERT_TRUE(tree.node(*subtree1).is_leaf());
  ++subtree1;
  auto subtree3 = tree.group(*subtree1).begin();
  ASSERT_TRUE(!tree.node(*subtree1).is_leaf());
  ++subtree1;
  ASSERT_TRUE(tree.node(*subtree1).is_leaf());

  ASSERT_TRUE(tree.node(*subtree2).is_leaf());
  ++subtree2;
  ASSERT_TRUE(tree.node(*subtree2).is_leaf());

  ASSERT_TRUE(tree.node(*subtree3).is_leaf());
  ++subtree3;
  ASSERT_TRUE(tree.node(*subtree3).is_leaf());
}

TEST_F(SDFTreeTest, LeavesAreCorrectlyUpdated) {
  // given
  //     o
  //   o   o
  //  o o o o
  resin::SDFTree tree;
  auto& group1 = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  auto g1p1    = group1.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union).node_id();
  auto g1p2    = group1.push_back_child<resin::SphereNode>(resin::SDFBinaryOperation::Union).node_id();
  auto& group2 = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Inter);
  auto g2p1    = group2.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Xor).node_id();
  auto g2p2    = group2.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Inter).node_id();

  // then
  //     4
  //   2   2
  //  * * * *
  ASSERT_EQ(tree.root().primitives().size(), 4);
  ASSERT_TRUE(tree.root().primitives().contains(g1p1));
  ASSERT_TRUE(tree.root().primitives().contains(g1p2));
  ASSERT_TRUE(tree.root().primitives().contains(g2p1));
  ASSERT_TRUE(tree.root().primitives().contains(g2p2));

  ASSERT_EQ(group1.primitives().size(), 2);
  ASSERT_TRUE(group1.primitives().contains(g1p1));
  ASSERT_TRUE(group1.primitives().contains(g1p2));

  ASSERT_EQ(group2.primitives().size(), 2);
  ASSERT_TRUE(group2.primitives().contains(g2p1));
  ASSERT_TRUE(group2.primitives().contains(g2p2));

  // and when
  auto g1p3 = group1.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union).node_id();
  tree.delete_node(group2.node_id());
  auto rp1 = tree.root().push_front_child<resin::CubeNode>(resin::SDFBinaryOperation::Union).node_id();
  auto rp2 = tree.root().push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union).node_id();

  // then
  //        5
  //   *    3    *
  //      * * *
  ASSERT_EQ(tree.root().primitives().size(), 5);
  ASSERT_TRUE(tree.root().primitives().contains(rp1));
  ASSERT_TRUE(tree.root().primitives().contains(rp2));
  ASSERT_TRUE(tree.root().primitives().contains(g1p1));
  ASSERT_TRUE(tree.root().primitives().contains(g1p2));
  ASSERT_TRUE(tree.root().primitives().contains(g1p3));

  ASSERT_EQ(group1.primitives().size(), 3);
  ASSERT_TRUE(group1.primitives().contains(g1p1));
  ASSERT_TRUE(group1.primitives().contains(g1p2));
  ASSERT_TRUE(group1.primitives().contains(g1p3));

  // and when
  auto node_ptr = tree.root().copy();
  auto group3   = node_ptr->node_id();
  group1.insert_before_child(g1p1, std::move(node_ptr));
  auto it        = tree.group(group3).begin();
  auto child_ptr = tree.group(group3).detach_child(*it);
  tree.root().push_back_child(std::move(child_ptr));
  it          = tree.group(group3).begin();
  auto group4 = *it;

  // then
  //                   10
  //    *         7         * *
  //         4  *   *  *
  //        3 *
  //      * * *
  ASSERT_EQ(tree.root().primitives().size(), 10);
  ASSERT_EQ(group1.primitives().size(), 7);
  ASSERT_EQ(tree.group(group3).primitives().size(), 4);
  ASSERT_EQ(tree.group(group4).primitives().size(), 3);
}

TEST_F(SDFTreeTest, DirtyPrimitivesAreCorrectlyAdded) {
  // given
  //       o
  //    o     o
  //  o   o o   o
  //           o o
  resin::SDFTree tree;
  auto& group1    = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  auto cube1_id   = group1.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union).node_id();
  auto sphere1_id = group1.push_back_child<resin::SphereNode>(resin::SDFBinaryOperation::Union).node_id();
  auto& group2    = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Inter);
  auto cube2_id   = group2.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Xor).node_id();
  auto& group3    = group2.push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Inter);
  auto cube3_id   = group3.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Xor).node_id();
  auto cube4_id   = group3.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Xor).node_id();

  // then
  const auto& dirty = tree.dirty_primitives();
  ASSERT_NE(std::find(dirty.begin(), dirty.end(), cube1_id), dirty.end());
  ASSERT_NE(std::find(dirty.begin(), dirty.end(), cube2_id), dirty.end());
  ASSERT_NE(std::find(dirty.begin(), dirty.end(), cube3_id), dirty.end());
  ASSERT_NE(std::find(dirty.begin(), dirty.end(), cube4_id), dirty.end());
  ASSERT_NE(std::find(dirty.begin(), dirty.end(), sphere1_id), dirty.end());

  // when
  tree.mark_primitives_clean();
  group2.mark_dirty();

  // then
  ASSERT_EQ(tree.dirty_primitives().size(), group2.primitives().size());
  std::vector<resin::IdView<resin::SDFTreeNodeId>> dirty_prims;
  for (auto elem : tree.dirty_primitives()) {
    dirty_prims.push_back(elem);
  }
  ASSERT_TRUE(std::is_permutation(dirty_prims.begin(), dirty_prims.end(), group2.primitives().begin()));
}

TEST_F(SDFTreeTest, MaterialsAreProperlyDerivedWhenMaterialSetOrRemoved) {
  // given
  //       o
  //    o     o
  //  o   o o   o
  //           o o
  resin::SDFTree tree;
  auto& group1 = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  group1.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union);
  group1.push_back_child<resin::SphereNode>(resin::SDFBinaryOperation::Union);
  auto& group2 = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Inter);
  group2.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Xor);
  auto& group3 = group2.push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Inter);
  group3.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Xor);
  group3.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Xor);

  auto& mat1 = tree.add_material(resin::Material(glm::vec3(1.F)));
  auto& mat2 = tree.add_material(resin::Material(glm::vec3(1.F)));

  // when
  group2.set_material(mat1.material_id());

  // then
  //
  //   materials        materials derivation
  //       o                    o
  //    o     1              o     1
  //  o   o o   o          o   o 1   1
  //           o o                  1 1
  //
  ASSERT_EQ(group2.material_id(), mat1.material_id());
  ASSERT_EQ(group2.ancestor_material_id(), std::nullopt);

  auto it = group2.begin();
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat1.material_id());
  ++it;
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat1.material_id());

  it = group3.begin();
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat1.material_id());
  ++it;
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat1.material_id());

  // when
  group3.set_material(mat2.material_id());

  // then
  //
  //   materials        materials derivation
  //       o                    o
  //    o     1              o     1
  //  o   o o   2          o   o 1   1
  //           o o                  1 1
  //
  ASSERT_EQ(group3.material_id(), mat2.material_id());

  it = group2.begin();
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat1.material_id());
  ++it;
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat1.material_id());

  it = group3.begin();
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat1.material_id());
  ++it;
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat1.material_id());

  // when
  group2.remove_material();

  // then
  //
  //   materials        materials derivation
  //       o                    o
  //    o     o              o     o
  //  o   o o   2          o   o o   2
  //           o o                  2 2
  //
  ASSERT_EQ(tree.root().ancestor_material_id(), std::nullopt);

  it = tree.root().begin();
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), std::nullopt);
  ++it;
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), std::nullopt);

  it = group1.begin();
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), std::nullopt);
  ++it;
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), std::nullopt);

  it = group2.begin();
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), std::nullopt);
  ++it;
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), std::nullopt);
  ASSERT_EQ(tree.node(*it).material_id(), mat2.material_id());
  it = group3.begin();
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat2.material_id());
  ++it;
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat2.material_id());
}

TEST_F(SDFTreeTest, MaterialsAreProperlyDeletedFromTree) {
  // given
  //
  //    materials          materials derivation
  //        2                      2
  //    1       3              2       2
  //  1 2 3   1   2          2 2 2   2   2
  //             2 1                    2 2
  //
  resin::SDFTree tree;
  auto mat1 = tree.add_material(resin::Material(glm::vec3(1.F))).material_id();
  auto mat2 = tree.add_material(resin::Material(glm::vec3(1.F))).material_id();
  auto mat3 = tree.add_material(resin::Material(glm::vec3(1.F))).material_id();

  tree.root().set_material(mat2);
  auto& group1 = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  group1.set_material(mat1);
  group1.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union).set_material(mat1);
  group1.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union).set_material(mat2);
  group1.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union).set_material(mat3);
  auto& group2 = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  group2.set_material(mat3);
  group2.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union).set_material(mat1);
  auto& group3 = group2.push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  group3.set_material(mat2);
  group3.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union).set_material(mat2);
  group3.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union).set_material(mat1);

  // when
  tree.delete_material(mat2);

  // then
  //
  //    materials         materials derivation
  //        o                      o
  //    1       3              1       3
  //  1 o 3   1   o          1 1 1   3   3
  //             o 1                    3 3
  //
  for (auto mat : tree.materials()) {
    ASSERT_NE(mat, mat2);
  }
  ASSERT_THROW(tree.material(mat2), resin::MaterialSDFTreeComponentDoesNotExist);

  ASSERT_EQ(tree.root().material_id(), std::nullopt);
  ASSERT_EQ(tree.root().ancestor_material_id(), std::nullopt);

  auto it = tree.root().begin();
  ASSERT_EQ(tree.node(*it).material_id(), mat1);
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), std::nullopt);
  ++it;
  ASSERT_EQ(tree.node(*it).material_id(), mat3);
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), std::nullopt);

  it = group1.begin();
  ASSERT_EQ(tree.node(*it).material_id(), mat1);
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat1);
  ++it;
  ASSERT_EQ(tree.node(*it).material_id(), std::nullopt);
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat1);
  ++it;
  ASSERT_EQ(tree.node(*it).material_id(), mat3);
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat1);

  it = group2.begin();
  ASSERT_EQ(tree.node(*it).material_id(), mat1);
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat3);
  ++it;
  ASSERT_FALSE(tree.node(*it).material_id().has_value());
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat3);

  it = group3.begin();
  ASSERT_EQ(tree.node(*it).material_id(), std::nullopt);
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat3);
  ++it;
  ASSERT_EQ(tree.node(*it).material_id(), mat1);
  ASSERT_EQ(tree.node(*it).ancestor_material_id(), mat3);
}
