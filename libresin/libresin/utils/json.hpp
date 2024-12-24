#ifndef RESIN_JSON_HPP
#define RESIN_JSON_HPP
#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <memory>
#include <nlohmann/json_fwd.hpp>

namespace resin {

constexpr int kNewestResinPrefabJSONSchemaVersion = 1;
constexpr StringEnumMapping<SDFTreePrimitiveType> kSDFTreePrimitiveNodesJSONNames({"sphere", "cube"});

struct Transform;
class MaterialSDFTreeComponent;
class SDFTreeNode;
class GroupNode;

class JSONSerializerSDFTreeNodeVisitor : public ISDFTreeNodeVisitor {
 public:
  explicit JSONSerializerSDFTreeNodeVisitor(nlohmann::json& json);

  void visit_sphere(SphereNode& node) override;
  void visit_cube(CubeNode& node) override;
  void visit_group(GroupNode& node) override;

 private:
  nlohmann::json& json_;  // NOLINT
};

void node_to_json(nlohmann::json& json, SDFTreeNode& node);
void transform_to_json(nlohmann::json& json, const Transform& transform);
void material_component_to_json(nlohmann::json& json, const MaterialSDFTreeComponent& material);
void sdf_tree_to_json(nlohmann::json& json, SDFTree& tree, IdView<SDFTreeNodeId> subtree_root_id,
                      bool ignore_unused_materials = true);
void sdf_tree_to_json(nlohmann::json& json, SDFTree& tree, bool ignore_unused_materials = true);

[[nodiscard]] std::string create_prefab_json(SDFTree& tree, IdView<SDFTreeNodeId> subtree_root_id);

class JSONDeserializerSDFTreeNodeVisitor : public ISDFTreeNodeVisitor {
 public:
  explicit JSONDeserializerSDFTreeNodeVisitor(const nlohmann::json& node_json,
                                              const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map);

  void visit_sphere(SphereNode& node) override;
  void visit_cube(CubeNode& node) override;
  void visit_group(GroupNode& node) override;

 private:
  const nlohmann::json& node_json_;
  const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map_;
};

[[nodiscard]] std::unique_ptr<GroupNode> parse_prefab_json(SDFTree& tree, std::string_view prefab_json);

}  // namespace resin

#endif
