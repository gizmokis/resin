#include "libresin/core/sdf_tree/sdf_tree.hpp"

#include <gtest/gtest.h>

#include <libresin/core/transform.hpp>
#include <print>
#include <tests/glm_helper.hpp>

#include "libresin/core/sdf_tree/group_node.hpp"
#include "libresin/core/sdf_tree/primitive_node.hpp"
#include "libresin/core/sdf_tree/sdf_tree_node.hpp"

class SDFTreeTest : public testing::Test {};

TEST_F(SDFTreeTest, SDFShaderIsCorrectlyGenerated) {}

TEST_F(SDFTreeTest, BasicOperationsWorkCorrectly) {}

TEST_F(SDFTreeTest, NodesAreCorrectlyMoved) {
  // given
  //       o
  //  o    o    o
  //      o o  o o
  resin::SDFTree tree;
  tree.root().push_back_child<resin::SphereNode>(resin::SDFBinaryOperation::Union);
  auto group1 = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Union).node_id();
  tree.group(group1).push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union).node_id();
  auto group1_child = tree.group(group1).push_back_child<resin::SphereNode>(resin::SDFBinaryOperation::Union).node_id();
  auto group2       = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Inter).node_id();
  tree.group(group2).push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Xor).node_id();
  tree.group(group2).push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Inter).node_id();

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

TEST_F(SDFTreeTest, NodesAreCorrectlyCopied) {}

TEST_F(SDFTreeTest, LeavesAreCorrectlyUpdatedOnDeleteAndInsert) {}

TEST_F(SDFTreeTest, DirtyPrimitivesAreCorrectlyAdded) {}
