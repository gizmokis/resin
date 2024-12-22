#ifndef RESIN_JSON_HPP
#define RESIN_JSON_HPP
#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <nlohmann/json_fwd.hpp>

namespace resin {
using json = nlohmann::json;

struct Transform;
class MaterialSDFTreeComponent;
class SDFTreeNode;
class GroupNode;

class JSONSerializerSDFTreeNodeVisitor : public ISDFTreeNodeVisitor {
 public:
  explicit JSONSerializerSDFTreeNodeVisitor(json& json);

  void visit_sphere(SphereNode& node) override;
  void visit_cube(CubeNode& node) override;
  void visit_group(GroupNode& node) override;

 private:
  json& json_;  // NOLINT
};

[[nodiscard]] json node_to_json(SDFTreeNode& node);
[[nodiscard]] json transform_to_json(const Transform& transform);
[[nodiscard]] json material_component_to_json(const MaterialSDFTreeComponent& material);
[[nodiscard]] json sdf_tree_to_json(SDFTree& tree, IdView<SDFTreeNodeId> subtree_root_id,
                                    bool ignore_unused_materials = true);
[[nodiscard]] json sdf_tree_to_json(SDFTree& tree, bool ignore_unused_materials = true);

}  // namespace resin

#endif
