#include <gtest/gtest.h>

#include <json_schemas/json_schemas.hpp>
#include <libresin/core/light.hpp>
#include <libresin/core/material.hpp>
#include <libresin/core/scene.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/transform.hpp>
#include <libresin/utils/json.hpp>
#include <tests/glm_helper.hpp>
#include <tests/random_helper.hpp>
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/adapters/std_string_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>

class JSONTest : public testing::Test {};

TEST_F(JSONTest, PrefabJSONSchemaIsValid) {
  auto schema_json    = nlohmann::json::parse(RESIN_PREFAB_JSON_SCHEMA);
  auto schema_adapter = valijson::adapters::NlohmannJsonAdapter(schema_json);
  auto schema         = valijson::Schema();
  auto schema_parser  = valijson::SchemaParser();

  ASSERT_NO_THROW(schema_parser.populateSchema(schema_adapter, schema));
}

TEST_F(JSONTest, ResinJSONSchemaIsValid) {
  auto schema_json    = nlohmann::json::parse(RESIN_SCENE_JSON_SCHEMA);
  auto schema_adapter = valijson::adapters::NlohmannJsonAdapter(schema_json);
  auto schema         = valijson::Schema();
  auto schema_parser  = valijson::SchemaParser();

  ASSERT_NO_THROW(schema_parser.populateSchema(schema_adapter, schema));
}

TEST_F(JSONTest, SerializedPrefabSatisfiesPrefabJSONSchema) {
  // given
  //       o
  //    o     o
  //  o   o o   o
  //           o o
  //
  resin::SDFTree tree;
  auto& mat1 = tree.add_material(resin::Material(glm::vec3(1.F)));
  auto& mat2 = tree.add_material(resin::Material(glm::vec3(1.F)));

  auto& group1 = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  group1.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union);
  group1.push_back_child<resin::SphereNode>(resin::SDFBinaryOperation::Union).set_material(mat2.material_id());
  auto& group2 = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Inter);
  group2.set_material(mat1.material_id());
  group2.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Xor);
  auto& group3 = group2.push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Inter);
  group3.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Xor);
  group3.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Xor);

  // when
  auto prefab_json_str = resin::json::serialize_prefab(tree, group2.node_id());
  auto prefab_json     = nlohmann::json::parse(prefab_json_str);
  auto prefab_adapter  = valijson::adapters::NlohmannJsonAdapter(prefab_json);

  auto schema_json    = nlohmann::json::parse(RESIN_PREFAB_JSON_SCHEMA);
  auto schema_adapter = valijson::adapters::NlohmannJsonAdapter(schema_json);

  auto schema        = valijson::Schema();
  auto schema_parser = valijson::SchemaParser();
  schema_parser.populateSchema(schema_adapter, schema);

  // then
  valijson::Validator validator;
  ASSERT_TRUE(validator.validate(schema, prefab_adapter, nullptr));
}

TEST_F(JSONTest, PrefabIsProperlySerializedAndDeserialized) {
  auto assert_nodes_common_eq = [](const resin::SDFTreeNode& node1, const resin::SDFTreeNode& node2) {
    EXPECT_GLM_VEC_NEAR(node1.transform().local_pos(), node2.transform().local_pos(), 1e-4F);
    EXPECT_GLM_ROT_NEAR(node1.transform().local_rot(), node2.transform().local_rot(), 1e-4F);
    EXPECT_NEAR(node1.transform().scale(), node2.transform().scale(), 1e-4F);
    ASSERT_EQ(node1.bin_op(), node2.bin_op());
    ASSERT_EQ(node1.name(), node2.name());
  };

  auto assert_materials_eq = [](const resin::MaterialSDFTreeComponent& material1,
                                const resin::MaterialSDFTreeComponent& material2) {
    EXPECT_GLM_VEC_NEAR(material1.material.albedo, material2.material.albedo, 1e-4F);
    EXPECT_NEAR(material1.material.ambientFactor, material2.material.ambientFactor, 1e-4F);
    EXPECT_NEAR(material1.material.diffuseFactor, material2.material.diffuseFactor, 1e-4F);
    EXPECT_NEAR(material1.material.specularFactor, material2.material.specularFactor, 1e-4F);
    EXPECT_NEAR(material1.material.specularExponent, material2.material.specularExponent, 1e-4F);
    ASSERT_EQ(material1.name(), material2.name());
  };

  // given
  //      o
  //   o      o
  //  o o  o  o  o
  //            o o
  //
  resin::SDFTree tree;
  auto& mat1 = tree.add_material(resin::Material(glm::vec3(1.F)));
  auto& mat2 = tree.add_material(resin::Material(glm::vec3(1.F)));
  randomize_material(mat1.material);
  randomize_material(mat2.material);

  auto& group1 = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  group1.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Union);
  group1.push_back_child<resin::SphereNode>(resin::SDFBinaryOperation::Union).set_material(mat2.material_id());
  auto& group2 = tree.root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Inter);
  group2.set_material(mat1.material_id());
  randomize_transform(group2.transform());
  randomize_transform(group2.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Xor).transform());
  randomize_transform(
      group2.push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Diff).transform());  // empty group
  auto& group3 = group2.push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Inter);
  randomize_transform(group3.transform());
  randomize_transform(group3.push_back_child<resin::SphereNode>(resin::SDFBinaryOperation::SmoothXor).transform());
  randomize_transform(group3.push_back_child<resin::CubeNode>(resin::SDFBinaryOperation::Xor).transform());

  // when
  auto prefab_json_str = resin::json::serialize_prefab(tree, group2.node_id());
  auto prefab          = resin::json::deserialize_prefab(tree, prefab_json_str);

  // then
  assert_nodes_common_eq(*prefab, group2);
  assert_materials_eq(tree.material(*prefab->material_id()), mat1);

  auto it        = group2.begin();
  auto prefab_it = prefab->begin();
  ASSERT_TRUE(tree.node(*it).is_leaf());
  ASSERT_EQ(prefab->get_child(*prefab_it).material_id(), std::nullopt);
  assert_nodes_common_eq(group2.get_child(*it), prefab->get_child(*prefab_it));
  ASSERT_NO_THROW({
    auto& prim        = static_cast<resin::CubeNode&>(group2.get_child(*it));          // NOLINT
    auto& prefab_prim = static_cast<resin::CubeNode&>(prefab->get_child(*prefab_it));  // NOLINT
    ASSERT_GLM_VEC_NEAR(prim.size, prefab_prim.size, 1e-4F);
  });

  it++;
  prefab_it++;
  assert_nodes_common_eq(tree.node(*it), prefab->get_child(*prefab_it));
  ASSERT_EQ(prefab->get_child(*prefab_it).material_id(), std::nullopt);
  ASSERT_TRUE(tree.node(*prefab_it).is_leaf());
  ASSERT_NO_THROW({
    static_cast<resin::GroupNode&>(tree.node(*prefab_it));  // NOLINT
  });

  it++;
  prefab_it++;
  assert_nodes_common_eq(tree.node(*it), prefab->get_child(*prefab_it));
  ASSERT_EQ(prefab->get_child(*prefab_it).material_id(), std::nullopt);
  ASSERT_FALSE(tree.node(*prefab_it).is_leaf());

  it        = group3.begin();
  prefab_it = tree.group(*prefab_it).begin();
  assert_nodes_common_eq(tree.node(*it), tree.node(*prefab_it));
  ASSERT_EQ(tree.node(*prefab_it).material_id(), std::nullopt);
  ASSERT_TRUE(tree.node(*prefab_it).is_leaf());
  ASSERT_NO_THROW({
    auto& prim        = static_cast<resin::SphereNode&>(tree.node(*it));         // NOLINT
    auto& prefab_prim = static_cast<resin::SphereNode&>(tree.node(*prefab_it));  // NOLINT
    ASSERT_NEAR(prim.radius, prefab_prim.radius, 1e-4F);
  });

  it++;
  prefab_it++;
  assert_nodes_common_eq(tree.node(*it), tree.node(*prefab_it));
  ASSERT_EQ(tree.node(*prefab_it).material_id(), std::nullopt);
  ASSERT_TRUE(tree.node(*prefab_it).is_leaf());
  ASSERT_NO_THROW({
    auto& prim        = static_cast<resin::CubeNode&>(tree.node(*it));         // NOLINT
    auto& prefab_prim = static_cast<resin::CubeNode&>(tree.node(*prefab_it));  // NOLINT
    ASSERT_GLM_VEC_NEAR(prim.size, prefab_prim.size, 1e-4F);
  });
}

TEST_F(JSONTest, SerializedSceneSatisfiesSceneJSONSchema) {
  // given
  //      o
  //      o
  //    o   o

  resin::Scene scene;
  scene.add_light<resin::DirectionalLight>();
  scene.add_light<resin::PointLight>();

  auto& mat = scene.tree().add_material(resin::Material(glm::vec3(1.F)));

  auto& group = scene.tree().root().push_back_child<resin::GroupNode>(resin::SDFBinaryOperation::Union);
  group.push_back_child<resin::TorusNode>(resin::SDFBinaryOperation::Union);
  group.push_back_child<resin::CylinderNode>(resin::SDFBinaryOperation::Union).set_material(mat.material_id());

  // when
  auto prefab_json_str = resin::json::serialize_scene(scene);
  auto prefab_json     = nlohmann::json::parse(prefab_json_str);
  auto prefab_adapter  = valijson::adapters::NlohmannJsonAdapter(prefab_json);

  auto schema_json    = nlohmann::json::parse(RESIN_PREFAB_JSON_SCHEMA);
  auto schema_adapter = valijson::adapters::NlohmannJsonAdapter(schema_json);

  auto schema        = valijson::Schema();
  auto schema_parser = valijson::SchemaParser();
  schema_parser.populateSchema(schema_adapter, schema);

  // then
  valijson::Validator validator;
  ASSERT_TRUE(validator.validate(schema, prefab_adapter, nullptr));
}

TEST_F(JSONTest, SceneLightsAreProperlySerializedAndDeserialized) {
  // given
  resin::Scene scene1;
  auto& dir_light = scene1.add_light<resin::DirectionalLight>(random_vec3(0.0F, 1.0F), random_float(0.0F, 1.0F));
  dir_light.rename("dir");
  auto& point_light = scene1.add_light<resin::PointLight>(
      random_vec3(0.0F, 1.0F), random_vec3(0.0F, 1.0F),
      resin::PointLight::Attenuation(random_float(0.0F, 1.0F), random_float(0.0F, 1.0F), random_float(0.0F, 1.0F)));
  point_light.rename("point");

  // when
  resin::Scene scene2;
  auto scene_json_str = resin::json::serialize_scene(scene1);
  resin::json::deserialize_scene(scene2, scene_json_str);

  // then
  ASSERT_EQ(scene1.lights().size(), scene2.lights().size());
  for (auto& it : scene2.lights()) {
    if (it.second->name() == "dir") {
      ASSERT_GLM_MAT_NEAR(it.second->light_base().transform.local_to_world_matrix(),
                          dir_light.light_base().transform.local_to_world_matrix(), 1e-4F);
      ASSERT_GLM_VEC_NEAR(it.second->light_base().color, dir_light.light_base().color, 1e-4F);

      auto* dir = reinterpret_cast<resin::DirectionalLight*>(&it.second->light_base());
      ASSERT_NEAR(dir->ambient_impact, dir_light.light().ambient_impact, 1e-4F);
    } else if (it.second->name() == "point") {
      ASSERT_GLM_MAT_NEAR(it.second->light_base().transform.local_to_world_matrix(),
                          point_light.light_base().transform.local_to_world_matrix(), 1e-4F);
      ASSERT_GLM_VEC_NEAR(it.second->light_base().color, point_light.light_base().color, 1e-4F);

      auto* point = reinterpret_cast<resin::PointLight*>(&it.second->light_base());
      ASSERT_NEAR(point->attenuation.linear, point_light.light().attenuation.linear, 1e-4F);
      ASSERT_NEAR(point->attenuation.constant, point_light.light().attenuation.constant, 1e-4F);
      ASSERT_NEAR(point->attenuation.quadratic, point_light.light().attenuation.quadratic, 1e-4F);
    }
  }
}
