#ifndef RESIN_JSON_HPP
#define RESIN_JSON_HPP
#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <memory>
#include <nlohmann/json_fwd.hpp>

#include "libresin/core/sdf_tree/primitive_base_node.hpp"

namespace resin {

struct Transform;
class MaterialSDFTreeComponent;
class SDFTreeNode;
class GroupNode;

namespace json {
using json = nlohmann::json;

constexpr int kNewestResinPrefabJSONSchemaVersion = 1;

constexpr StringEnumMapping<SDFTreePrimitiveType> kSDFTreePrimitiveNodesJSONNames({
    {SDFTreePrimitiveType::Sphere, "sphere"},  //
    {SDFTreePrimitiveType::Cube, "cube"}       //
});

constexpr StringEnumMapping<SDFBinaryOperation> kSDFBinaryOperationsJSONNames({
    {SDFBinaryOperation::Union, "union"},              //
    {SDFBinaryOperation::SmoothUnion, "smoothUnion"},  //
    {SDFBinaryOperation::Diff, "diff"},                //
    {SDFBinaryOperation::SmoothDiff, "smoothDiff"},    //
    {SDFBinaryOperation::Inter, "inter"},              //
    {SDFBinaryOperation::SmoothInter, "smoothInter"},  //
    {SDFBinaryOperation::Xor, "xor"},                  //
    {SDFBinaryOperation::SmoothXor, "smoothXor"},      //
});

void serialize_transform(json& target_json, const Transform& transform);
void serialize_material(json& target_json, const MaterialSDFTreeComponent& material);

void serialize_node_material(json& target_json, const SDFTreeNode& node);
void serialize_node_name(json& target_json, const SDFTreeNode& node);
void serialize_node_bin_op(json& target_json, const SDFTreeNode& node);

// Serializes material id, name, transform and binary operation
void serialize_node_common(json& target_json, const SDFTreeNode& node);

// Serializes node specific data
class JSONSerializerSDFTreeNodeVisitor : public ISDFTreeNodeVisitor {
 public:
  explicit JSONSerializerSDFTreeNodeVisitor(json& json);

  void visit_sphere(SphereNode& node) override;
  void visit_cube(CubeNode& node) override;
  void visit_group(GroupNode& node) override;

 private:
  json& json_;
};

void serialize_sdf_tree(json& target_json, const SDFTree& tree, IdView<SDFTreeNodeId> subtree_root_id,
                        bool ignore_unused_materials = true);
void serialize_sdf_tree(json& target_json, const SDFTree& tree, bool ignore_unused_materials = true);

[[nodiscard]] std::string serialize_prefab(const SDFTree& tree, IdView<SDFTreeNodeId> subtree_root_id);

void deserialize_transform(Transform& transform, const json& trans_json);
void deserialize_material(MaterialSDFTreeComponent& material, const json& mat_json);

void deserialize_node_material(SDFTreeNode& node, const json& node_json,
                               const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map);
void deserialize_node_name(SDFTreeNode& node, const json& node_json);
void deserialize_node_bin_op(SDFTreeNode& node, const json& node_json);

// Deserializes material id, name, transform and binary operation
void deserialize_node_common(SDFTreeNode& node, const json& node_json,
                             const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map);

// Deserializes node specific data
class JSONDeserializerSDFTreeNodeVisitor : public ISDFTreeNodeVisitor {
 public:
  explicit JSONDeserializerSDFTreeNodeVisitor(const json& node_json,
                                              const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map);

  void visit_sphere(SphereNode& node) override;
  void visit_cube(CubeNode& node) override;
  void visit_group(GroupNode& node) override;

 private:
  const json& node_json_;
  const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map_;
};

[[nodiscard]] std::unique_ptr<GroupNode> deserialize_prefab(SDFTree& tree, std::string_view prefab_json_str);

}  // namespace json

}  // namespace resin
#endif
