#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/json.hpp>
#include <libresin/utils/logger.hpp>
#include <nlohmann/json.hpp>
#include <unordered_set>

namespace resin {

JSONSerializerSDFTreeNodeVisitor::JSONSerializerSDFTreeNodeVisitor(nlohmann::json& json) : json_(json) {}

void JSONSerializerSDFTreeNodeVisitor::visit_group(GroupNode& node) {
  if (node.material_id().has_value()) {
    json_["group"]["materialId"] = node.material_id()->raw();
  }
  json_["group"]["name"] = node.name();
  transform_to_json(json_["group"], node.transform());
  auto children = nlohmann::json::array();
  for (auto child : node) {
    nlohmann::json child_json;
    JSONSerializerSDFTreeNodeVisitor visitor(child_json);
    node.get_child(child).accept_visitor(visitor);
    children.push_back(child_json);
  }
  json_["group"]["children"] = children;
}

void JSONSerializerSDFTreeNodeVisitor::visit_cube(CubeNode& node) {
  json_["cube"]["name"] = node.name();
  transform_to_json(json_["cube"], node.transform());
  if (node.material_id().has_value() && !node.is_material_default()) {
    json_["cube"]["materialId"] = node.material_id()->raw();
  }
  json_["cube"]["size"]["x"] = node.size;
  json_["cube"]["size"]["y"] = node.size;
  json_["cube"]["size"]["z"] = node.size;
}

void JSONSerializerSDFTreeNodeVisitor::visit_sphere(SphereNode& node) {
  json_["sphere"]["name"] = node.name();
  transform_to_json(json_["sphere"], node.transform());
  if (node.material_id().has_value() && !node.is_material_default()) {
    json_["sphere"]["materialId"] = node.material_id()->raw();
  }
  json_["sphere"]["radius"] = node.radius;
}

void node_to_json(nlohmann::json& json, SDFTreeNode& node) {
  JSONSerializerSDFTreeNodeVisitor visitor(json);
  node.accept_visitor(visitor);
}

void transform_to_json(nlohmann::json& json, const Transform& transform) {
  json["transform"]["position"]["x"] = transform.pos().x;
  json["transform"]["position"]["y"] = transform.pos().y;
  json["transform"]["position"]["z"] = transform.pos().z;

  json["transform"]["rotation"]["x"] = transform.rot().x;
  json["transform"]["rotation"]["y"] = transform.rot().y;
  json["transform"]["rotation"]["z"] = transform.rot().z;
  json["transform"]["rotation"]["w"] = transform.rot().w;

  json["transform"]["scale"] = transform.scale();
}

void material_component_to_json(nlohmann::json& json, const MaterialSDFTreeComponent& material) {
  json["material"]["id"]               = material.material_id().raw();
  json["material"]["name"]             = material.name();
  json["material"]["albedo"]["r"]      = material.material.albedo.r;
  json["material"]["albedo"]["g"]      = material.material.albedo.g;
  json["material"]["albedo"]["b"]      = material.material.albedo.b;
  json["material"]["ambient"]          = material.material.ambientFactor;
  json["material"]["diffuse"]          = material.material.diffuseFactor;
  json["material"]["specular"]         = material.material.specularFactor;
  json["material"]["specularExponent"] = material.material.specularExponent;
}

static void find_used_materials(
    std::unordered_set<IdView<MaterialId>, IdViewHash<MaterialId>, std::equal_to<>>& used_materials, SDFTree& tree,
    IdView<SDFTreeNodeId> subtree_root_id) {
  if (tree.is_group(subtree_root_id)) {
    for (const auto& child_id : tree.group(subtree_root_id)) {
      find_used_materials(used_materials, tree, child_id);
    }
  }

  auto mat_id = tree.node(subtree_root_id).material_id();
  if (mat_id.has_value() && *mat_id != tree.default_material().material_id()) {
    used_materials.insert(*mat_id);
  }
}

void sdf_tree_to_json(nlohmann::json& json, SDFTree& tree, IdView<SDFTreeNodeId> subtree_root_id,
                      bool ignore_unused_materials) {
  node_to_json(json["tree"], tree.node(subtree_root_id));
  auto materials = nlohmann::json::array();

  if (ignore_unused_materials) {
    std::unordered_set<IdView<MaterialId>, IdViewHash<MaterialId>, std::equal_to<>> used_materials;
    find_used_materials(used_materials, tree, subtree_root_id);
    for (auto mat : used_materials) {
      nlohmann::json mat_json;
      material_component_to_json(mat_json, tree.material(mat));
      materials.push_back(mat_json["material"]);
    }
  } else {  // NOLINT
    for (auto mat : tree.materials()) {
      nlohmann::json mat_json;
      material_component_to_json(mat_json, tree.material(mat));
      materials.push_back(mat_json["material"]);
    }
  }
  json["tree"]["materials"] = materials;
}

void sdf_tree_to_json(nlohmann::json& json, SDFTree& tree, bool ignore_unused_materials) {
  sdf_tree_to_json(json, tree, tree.root().node_id(), ignore_unused_materials);
}

std::string create_prefab_json(SDFTree& tree, IdView<SDFTreeNodeId> subtree_root_id) {
  nlohmann::json prefab_json;
  prefab_json["version"] = kNewestResinPrefabJSONSchemaVersion;
  sdf_tree_to_json(prefab_json, tree, subtree_root_id, true);
  return prefab_json.dump(2);
}

}  // namespace resin
