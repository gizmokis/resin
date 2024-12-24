#include <glm/fwd.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/json.hpp>
#include <libresin/utils/logger.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <unordered_set>

namespace resin {

static bool property_exists(const nlohmann::json& j, std::string_view key) { return j.find(key) != j.end(); }

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

static void add_transform_to_node(SDFTreeNode& node, const nlohmann::json& node_json) {
  node.transform().set_local_pos(
      glm::vec3(node_json["transform"]["x"], node_json["transform"]["y"], node_json["transform"]["z"]));
  node.transform().set_local_rot(
      glm::quat(node_json["transform"]["rotation"]["x"], node_json["transform"]["rotation"]["y"],
                node_json["transform"]["rotation"]["z"], node_json["transform"]["rotation"]["w"]));
  node.transform().set_local_scale(node_json["transform"]["scale"]);
}

static void add_name_to_node(SDFTreeNode& node, const nlohmann::json& node_json) { node.rename(node_json["name"]); }

static void add_material_to_node(SDFTreeNode& node, const nlohmann::json& node_json,
                                 const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map) {
  if (!property_exists(node_json, "materialId")) {
    return;
  }

  size_t id   = node_json["materialId"];
  auto mat_it = material_ids_map.find(id);
  if (mat_it == material_ids_map.end()) {
    log_throw(JSONDeserializationException(
        std::format("Node with name references to non existing material with id {}.", id)));
  }

  node.set_material(mat_it->second);
}

static void add_data_to_node(SDFTreeNode& node, const nlohmann::json& node_json,
                             const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map) {
  add_material_to_node(node, node_json, material_ids_map);
  add_name_to_node(node, node_json);
  add_transform_to_node(node, node_json);
}

JSONDeserializerSDFTreeNodeVisitor::JSONDeserializerSDFTreeNodeVisitor(
    const nlohmann::json& node_json, const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map)
    : node_json_(node_json), material_ids_map_(material_ids_map) {}

void JSONDeserializerSDFTreeNodeVisitor::visit_sphere(SphereNode& node) { node.radius = node_json_["radius"]; }

void JSONDeserializerSDFTreeNodeVisitor::visit_cube(CubeNode& node) {
  node.size = node_json_["size"]["x"];  // TODO(SDF-): update json deserializer for node
}

void JSONDeserializerSDFTreeNodeVisitor::visit_group(GroupNode& node) {
  for (const auto& child_json : node_json_["children"]) {
    for (auto [prim_type, name] : kSDFTreePrimitiveNodesJSONNames) {
      if (property_exists(child_json, name)) {
        auto& child_prim = node.push_back_primitive(prim_type, SDFBinaryOperation::Union);

        add_data_to_node(child_prim, child_json[name], material_ids_map_);
        auto visitor = JSONDeserializerSDFTreeNodeVisitor(child_json[name], material_ids_map_);
        child_prim.accept_visitor(visitor);
      }
    }

    if (property_exists(child_json, "group")) {
      auto& child_group = node.push_back_child<GroupNode>(SDFBinaryOperation::Union);

      add_data_to_node(child_group, child_json["group"], material_ids_map_);
      auto visitor = JSONDeserializerSDFTreeNodeVisitor(child_json["group"], material_ids_map_);
      child_group.accept_visitor(visitor);
    }
  }
}

std::unique_ptr<GroupNode> parse_prefab_json(SDFTree& tree, std::string_view prefab_json) {
  auto json = nlohmann::json::parse(prefab_json)["tree"];

  std::unordered_map<size_t, IdView<MaterialId>> material_ids_map;
  for (const auto& mat_json : json["materials"]) {
    auto& mat = tree.add_material(
        Material(glm::vec3(mat_json["albedo"]["r"], mat_json["albedo"]["g"], mat_json["albedo"]["b"]),
                 mat_json["ambient"], mat_json["diffuse"], mat_json["specular"], mat_json["specularExponent"]));
    mat.rename(mat_json["name"]);

    auto mat_it = material_ids_map.find(mat_json["id"]);
    if (mat_it != material_ids_map.end()) {
      log_throw(JSONDeserializationException(
          std::format("More than one definition of a material with id {} found.", mat_it->first)));
    }

    material_ids_map[mat_json["id"]] = mat.material_id();
  }

  auto prefab_root = tree.create_detached_node<GroupNode>();
  add_data_to_node(*prefab_root, json["group"], material_ids_map);
  auto visitor = JSONDeserializerSDFTreeNodeVisitor(json["group"], material_ids_map);
  prefab_root->accept_visitor(visitor);

  return prefab_root;
}

}  // namespace resin
