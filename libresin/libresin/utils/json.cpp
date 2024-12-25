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

namespace json {

static bool property_exists(const json& j, std::string_view key) { return j.find(key) != j.end(); }

void serialize_transform(json& target_json, const Transform& transform) {
  target_json["position"]["x"] = transform.pos().x;
  target_json["position"]["y"] = transform.pos().y;
  target_json["position"]["z"] = transform.pos().z;

  target_json["rotation"]["x"] = transform.rot().x;
  target_json["rotation"]["y"] = transform.rot().y;
  target_json["rotation"]["z"] = transform.rot().z;
  target_json["rotation"]["w"] = transform.rot().w;

  target_json["scale"] = transform.scale();
}

void serialize_material(json& target_json, const MaterialSDFTreeComponent& material) {
  target_json["id"]               = material.material_id().raw();
  target_json["name"]             = material.name();
  target_json["albedo"]["r"]      = material.material.albedo.r;
  target_json["albedo"]["g"]      = material.material.albedo.g;
  target_json["albedo"]["b"]      = material.material.albedo.b;
  target_json["ambient"]          = material.material.ambientFactor;
  target_json["diffuse"]          = material.material.diffuseFactor;
  target_json["specular"]         = material.material.specularFactor;
  target_json["specularExponent"] = material.material.specularExponent;
}

void serialize_node_material(json& target_json, const SDFTreeNode& node) {
  if (node.material_id().has_value()) {
    target_json["materialId"] = node.material_id()->raw();
  }
}

void serialize_node_name(json& target_json, const SDFTreeNode& node) { target_json["name"] = node.name(); }

void serialize_bin_op(json& target_json, const SDFTreeNode& node) {
  target_json["binaryOperation"] = kSDFBinaryOperationsJSONNames[node.bin_op()];
}

void serialize_node_common(json& target_json, const SDFTreeNode& node) {
  serialize_transform(target_json["transform"], node.transform());

  serialize_node_material(target_json, node);
  serialize_node_name(target_json, node);
  serialize_node_bin_op(target_json, node);
}

JSONSerializerSDFTreeNodeVisitor::JSONSerializerSDFTreeNodeVisitor(json& json) : json_(json) {}

void JSONSerializerSDFTreeNodeVisitor::visit_group(GroupNode& node) {
  auto children = json::array();
  for (auto child_id : node) {
    auto& child = node.get_child(child_id);
    json child_json;
    JSONSerializerSDFTreeNodeVisitor visitor(child_json);
    serialize_node_common(child_json, child);
    child.accept_visitor(visitor);
    children.push_back(child_json);
  }
  json_["group"]["children"] = children;
}

void JSONSerializerSDFTreeNodeVisitor::visit_cube(CubeNode& node) {
  json_["cube"]["size"]["x"] = node.size;
  json_["cube"]["size"]["y"] = node.size;
  json_["cube"]["size"]["z"] = node.size;
}

void JSONSerializerSDFTreeNodeVisitor::visit_sphere(SphereNode& node) { json_["sphere"]["radius"] = node.radius; }

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

void serialzie_sdf_tree(json& target_json, SDFTree& tree, IdView<SDFTreeNodeId> subtree_root_id,
                        bool ignore_unused_materials) {
  auto materials = json::array();
  if (ignore_unused_materials) {
    std::unordered_set<IdView<MaterialId>, IdViewHash<MaterialId>, std::equal_to<>> used_materials;
    find_used_materials(used_materials, tree, subtree_root_id);
    for (auto mat : used_materials) {
      json mat_json;
      serialize_material(mat_json, tree.material(mat));
      materials.push_back(mat_json["material"]);
    }
  } else {
    for (auto mat : tree.materials()) {
      json mat_json;
      serialize_material(mat_json, tree.material(mat));
      materials.push_back(mat_json["material"]);
    }
  }
  target_json["tree"]["materials"] = materials;

  auto& root_group = tree.group(subtree_root_id);
  serialize_node_common(target_json["tree"]["rootGroup"], root_group);
  auto visitor = JSONSerializerSDFTreeNodeVisitor(target_json["tree"]["rootGroup"]);
  root_group.accept_visitor(visitor);
}

void serialzie_sdf_tree(json& target_json, const SDFTree& tree, bool ignore_unused_materials) {
  serialzie_sdf_tree(target_json, tree, tree.root().node_id(), ignore_unused_materials);
}

std::string serialize_prefab(SDFTree& tree, IdView<SDFTreeNodeId> subtree_root_id) {
  json prefab_json;
  prefab_json["version"] = kNewestResinPrefabJSONSchemaVersion;
  serialzie_sdf_tree(prefab_json, tree, subtree_root_id, true);
  return prefab_json.dump(2);
}

void deserialize_node_material(SDFTreeNode& node, const json& node_json,
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

void deserialize_node_name(SDFTreeNode& node, const json& node_json) { node.rename(node_json["name"]); }

void deserialize_transform(Transform& transform, const json& node_json) {
  transform.set_local_pos(
      glm::vec3(node_json["position"]["x"], node_json["position"]["y"], node_json["position"]["z"]));
  transform.set_local_rot(glm::quat(node_json["rotation"]["x"], node_json["rotation"]["y"], node_json["rotation"]["z"],
                                    node_json["rotation"]["w"]));
  transform.set_local_scale(node_json["scale"]);
}

void deserialize_material(MaterialSDFTreeComponent& material, const json& mat_json) {
  material.material =
      Material(glm::vec3(mat_json["albedo"]["r"], mat_json["albedo"]["g"], mat_json["albedo"]["b"]),
               mat_json["ambient"], mat_json["diffuse"], mat_json["specular"], mat_json["specularExponent"]);
  material.rename(mat_json["name"]);
}

void deserialize_node_bin_op(SDFTreeNode& node, const json& node_json) {
  for (auto [op, name] : kSDFBinaryOperationsJSONNames) {
    if (node_json["binaryOperation"] == name) {
      node.set_bin_op(op);
      break;
    }
  }
}

void deserialize_node_common(SDFTreeNode& node, const json& node_json,
                             const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map) {
  deserialize_transform(node.transform(), node_json["transform"]);

  deserialize_node_material(node, node_json, material_ids_map);
  deserialize_node_name(node, node_json);
  deserialize_node_bin_op(node, node_json);
}

JSONDeserializerSDFTreeNodeVisitor::JSONDeserializerSDFTreeNodeVisitor(
    const json& node_json, const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map)
    : node_json_(node_json), material_ids_map_(material_ids_map) {}

void JSONDeserializerSDFTreeNodeVisitor::visit_sphere(SphereNode& node) {
  node.radius = node_json_["sphere"]["radius"];
}

void JSONDeserializerSDFTreeNodeVisitor::visit_cube(CubeNode& node) {
  node.size = node_json_["cube"]["size"]["x"];  // TODO(SDF-): update json deserializer for node
}

void JSONDeserializerSDFTreeNodeVisitor::visit_group(GroupNode& node) {
  for (const auto& child_json : node_json_["group"]["children"]) {
    for (auto [prim_type, name] : kSDFTreePrimitiveNodesJSONNames) {
      if (property_exists(child_json, name)) {
        auto& child_prim = node.push_back_primitive(prim_type, SDFBinaryOperation::Union);

        deserialize_node_common(child_prim, child_json, material_ids_map_);
        auto visitor = JSONDeserializerSDFTreeNodeVisitor(child_json, material_ids_map_);
        child_prim.accept_visitor(visitor);

        break;
      }
    }

    if (property_exists(child_json, "group")) {
      auto& child_group = node.push_back_child<GroupNode>(SDFBinaryOperation::Union);

      deserialize_node_common(child_group, child_json, material_ids_map_);
      auto visitor = JSONDeserializerSDFTreeNodeVisitor(child_json, material_ids_map_);
      child_group.accept_visitor(visitor);
    }
  }
}

std::unique_ptr<GroupNode> deserialize_prefab(SDFTree& tree, std::string_view prefab_json) {
  auto json = json::parse(prefab_json)["tree"];

  std::unordered_map<size_t, IdView<MaterialId>> material_ids_map;
  for (const auto& mat_json : json["materials"]) {
    auto& mat = tree.add_material(Material());
    deserialize_material(mat, mat_json);

    auto mat_it = material_ids_map.find(mat_json["id"]);
    if (mat_it != material_ids_map.end()) {
      log_throw(JSONDeserializationException(
          std::format("More than one definition of a material with id {} found.", mat_it->first)));
    }

    material_ids_map[mat_json["id"]] = mat.material_id();
  }

  auto prefab_root = tree.create_detached_node<GroupNode>();
  deserialize_node_common(*prefab_root, json["rootGroup"], material_ids_map);
  auto visitor = JSONDeserializerSDFTreeNodeVisitor(json["rootGroup"], material_ids_map);
  prefab_root->accept_visitor(visitor);

  return prefab_root;
}

}  // namespace json

}  // namespace resin
