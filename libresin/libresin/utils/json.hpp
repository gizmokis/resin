#ifndef RESIN_JSON_HPP
#define RESIN_JSON_HPP
#include <libresin/core/id_registry.hpp>
#include <libresin/core/light.hpp>
#include <libresin/core/scene.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <memory>
#include <nlohmann/json_fwd.hpp>

namespace resin {

struct Transform;
class MaterialSDFTreeComponent;
class SDFTreeNode;
class GroupNode;

namespace json {
using json = nlohmann::json;

constexpr int kNewestResinPrefabJSONSchemaVersion = 1;

constexpr StringEnumMapper<SDFTreePrimitiveType> kSDFTreePrimitiveNodesJSONNames({
    {SDFTreePrimitiveType::Sphere, "sphere"},                   //
    {SDFTreePrimitiveType::Cube, "cube"},                       //
    {SDFTreePrimitiveType::Torus, "torus"},                     //
    {SDFTreePrimitiveType::Capsule, "capsule"},                 //
    {SDFTreePrimitiveType::Link, "link"},                       //
    {SDFTreePrimitiveType::Ellipsoid, "ellipsoid"},             //
    {SDFTreePrimitiveType::Pyramid, "pyramid"},                 //
    {SDFTreePrimitiveType::Cylinder, "cylinder"},               //
    {SDFTreePrimitiveType::TriangularPrism, "triangularPrism"}  //
});

constexpr StringEnumMapper<SDFBinaryOperation> kSDFBinaryOperationsJSONNames({
    {SDFBinaryOperation::Union, "union"},              //
    {SDFBinaryOperation::SmoothUnion, "smoothUnion"},  //
    {SDFBinaryOperation::Diff, "diff"},                //
    {SDFBinaryOperation::SmoothDiff, "smoothDiff"},    //
    {SDFBinaryOperation::Inter, "inter"},              //
    {SDFBinaryOperation::SmoothInter, "smoothInter"},  //
    {SDFBinaryOperation::Xor, "xor"},                  //
    {SDFBinaryOperation::SmoothXor, "smoothXor"},      //
});

constexpr StringEnumMapper<LightType> kLightJSONNames({
    {LightType::DirectionalLight, "directionalLight"},  //
    {LightType::PointLight, "pointLight"},              //
});

// Serializes node specific data
class JSONSerializerSDFTreeNodeVisitor : public ISDFTreeNodeVisitor {
 public:
  explicit JSONSerializerSDFTreeNodeVisitor(json& node_json);

  void visit_group(GroupNode& node) override;
  void visit_sphere(SphereNode& node) override;
  void visit_cube(CubeNode& node) override;
  void visit_torus(TorusNode&) override;
  void visit_capsule(CapsuleNode&) override;
  void visit_link(LinkNode&) override;
  void visit_ellipsoid(EllipsoidNode&) override;
  void visit_pyramid(PyramidNode&) override;
  void visit_cylinder(CylinderNode&) override;
  void visit_prism(TriangularPrismNode&) override;

 private:
  json& json_;  // NOLINT
};

class JSONSerializerLightSceneComponentVisitor : public ILightSceneComponentVisitor {
 public:
  explicit JSONSerializerLightSceneComponentVisitor(json& light_json);

  void visit_point_light(LightSceneComponent<PointLight>& point_light) override;
  void visit_directional_light(LightSceneComponent<DirectionalLight>& dir_light) override;

 private:
  json& json_;  // NOLINT
};

void serialize_transform(json& target_json, const Transform& transform);
void serialize_material(json& target_json, const MaterialSDFTreeComponent& material);

void serialize_node_material(json& target_json, const SDFTreeNode& node);
void serialize_node_name(json& target_json, const SDFTreeNode& node);
void serialize_node_bin_op(json& target_json, const SDFTreeNode& node);
void serialize_node_factor(json& target_json, const SDFTreeNode& node);

// Serializes material id, name, transform and binary operation
void serialize_node_common(json& target_json, const SDFTreeNode& node);

void serialize_sdf_tree(json& target_json, SDFTree& tree, IdView<SDFTreeNodeId> subtree_root_id,
                        bool ignore_unused_materials = true);
void serialize_sdf_tree(json& target_json, SDFTree& tree, bool ignore_unused_materials = true);

void serialize_light_common(json& target_json, const BaseLightSceneComponent& light);
void serialize_attenuation(json& target_json, const PointLight::Attenuation& attenuation);

[[nodiscard]] std::string serialize_prefab(SDFTree& tree, IdView<SDFTreeNodeId> subtree_root_id);
[[nodiscard]] std::string serialize_scene(Scene& scene);

// Deserializes node specific data
class JSONDeserializerSDFTreeNodeVisitor : public ISDFTreeNodeVisitor {
 public:
  explicit JSONDeserializerSDFTreeNodeVisitor(const json& node_json,
                                              const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map);

  void visit_group(GroupNode& node) override;
  void visit_sphere(SphereNode& node) override;
  void visit_cube(CubeNode& node) override;
  void visit_torus(TorusNode&) override;
  void visit_capsule(CapsuleNode&) override;
  void visit_link(LinkNode&) override;
  void visit_ellipsoid(EllipsoidNode&) override;
  void visit_pyramid(PyramidNode&) override;
  void visit_cylinder(CylinderNode&) override;
  void visit_prism(TriangularPrismNode&) override;

 private:
  const json& node_json_;                                                   // NOLINT
  const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map_;  // NOLINT
};

class JSONDeserializerLightSceneComponentVisitor : public ILightSceneComponentVisitor {
 public:
  explicit JSONDeserializerLightSceneComponentVisitor(const json& light_json);

  void visit_point_light(LightSceneComponent<PointLight>& point_light) override;
  void visit_directional_light(LightSceneComponent<DirectionalLight>& dir_light) override;

 private:
  const json& light_json_;  // NOLINT
};

void deserialize_transform(Transform& transform, const json& trans_json);
void deserialize_material(MaterialSDFTreeComponent& material, const json& mat_json);

void deserialize_node_material(SDFTreeNode& node, const json& node_json,
                               const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map);
void deserialize_node_name(SDFTreeNode& node, const json& node_json);
void deserialize_node_bin_op(SDFTreeNode& node, const json& node_json);
void deserialize_node_factor(SDFTreeNode& node, const json& node_json);

// Deserializes material id, name, transform and binary operation
void deserialize_node_common(SDFTreeNode& node, const json& node_json,
                             const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map);

void deserialize_light_common(BaseLightSceneComponent& light, const json& light_json);
void deserialize_attenuation(PointLight::Attenuation& attenuation, const json& attenuation_json);

std::unique_ptr<GroupNode> deserialize_sdf_tree(SDFTree& tree, const json& tree_json);

[[nodiscard]] std::unique_ptr<GroupNode> deserialize_prefab(SDFTree& tree, std::string_view prefab_json_str);
void deserialize_scene(Scene& scene, std::string_view scene_json_str);

}  // namespace json

}  // namespace resin
#endif
