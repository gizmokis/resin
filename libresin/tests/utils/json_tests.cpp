#include <gtest/gtest.h>

#include <json_schemas/json_schemas.hpp>
#include <libresin/core/material.hpp>
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
