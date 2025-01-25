#include <glm/fwd.hpp>
#include <json_schemas/json_schemas.hpp>
#include <libresin/core/light.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/json.hpp>
#include <libresin/utils/logger.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <unordered_set>

namespace resin {

namespace json {

namespace {

bool property_exists(const json& j, std::string_view key) { return j.find(key) != j.end(); }

void find_used_materials(
    std::unordered_set<IdView<MaterialId>, IdViewHash<MaterialId>, std::equal_to<>>& used_materials,
    const SDFTree& tree, IdView<SDFTreeNodeId> subtree_root_id) {
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

}  // namespace

void serialize_transform(json& target_json, const Transform& transform) {
  target_json["position"]["x"] = transform.local_pos().x;
  target_json["position"]["y"] = transform.local_pos().y;
  target_json["position"]["z"] = transform.local_pos().z;

  target_json["rotation"]["w"] = transform.local_rot().w;
  target_json["rotation"]["x"] = transform.local_rot().x;
  target_json["rotation"]["y"] = transform.local_rot().y;
  target_json["rotation"]["z"] = transform.local_rot().z;

  target_json["scale"] = transform.local_scale();
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

void serialize_node_bin_op(json& target_json, const SDFTreeNode& node) {
  target_json["binaryOperation"] = kSDFBinaryOperationsJSONNames[node.bin_op()];
}

void serialize_node_factor(json& target_json, const SDFTreeNode& node) { target_json["factor"] = node.factor(); }

void serialize_node_common(json& target_json, const SDFTreeNode& node) {
  serialize_transform(target_json["transform"], node.transform());

  serialize_node_material(target_json, node);
  serialize_node_name(target_json, node);
  serialize_node_bin_op(target_json, node);
  serialize_node_factor(target_json, node);
}

JSONSerializerSDFTreeNodeVisitor::JSONSerializerSDFTreeNodeVisitor(json& node_json) : json_(node_json) {}

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

void JSONSerializerSDFTreeNodeVisitor::visit_sphere(SphereNode& node) { json_["sphere"]["radius"] = node.radius; }

void JSONSerializerSDFTreeNodeVisitor::visit_cube(CubeNode& node) {
  json_["cube"]["size"]["x"] = node.size.x;
  json_["cube"]["size"]["y"] = node.size.y;
  json_["cube"]["size"]["z"] = node.size.z;
}

void JSONSerializerSDFTreeNodeVisitor::visit_torus(TorusNode& node) {
  json_["torus"]["majorRadius"] = node.major_radius;
  json_["torus"]["minorRadius"] = node.minor_radius;
}

void JSONSerializerSDFTreeNodeVisitor::visit_capsule(CapsuleNode& node) {
  json_["capsule"]["height"] = node.height;
  json_["capsule"]["radius"] = node.radius;
}

void JSONSerializerSDFTreeNodeVisitor::visit_link(LinkNode& node) {
  json_["link"]["length"]      = node.length;
  json_["link"]["majorRadius"] = node.major_radius;
  json_["link"]["minorRadius"] = node.minor_radius;
}

void JSONSerializerSDFTreeNodeVisitor::visit_ellipsoid(EllipsoidNode& node) {
  json_["ellipsoid"]["radii"]["x"] = node.radii.x;
  json_["ellipsoid"]["radii"]["y"] = node.radii.y;
  json_["ellipsoid"]["radii"]["z"] = node.radii.z;
}

void JSONSerializerSDFTreeNodeVisitor::visit_pyramid(PyramidNode& node) { json_["pyramid"]["height"] = node.height; }

void JSONSerializerSDFTreeNodeVisitor::visit_cylinder(CylinderNode& node) {
  json_["cylinder"]["height"] = node.height;
  json_["cylinder"]["radius"] = node.radius;
}

void JSONSerializerSDFTreeNodeVisitor::visit_prism(TriangularPrismNode& node) {
  json_["triangularPrism"]["prismHeight"] = node.prismHeight;
  json_["triangularPrism"]["baseHeight"]  = node.baseHeight;
}

void serialize_sdf_tree(json& target_json, SDFTree& tree, IdView<SDFTreeNodeId> subtree_root_id,
                        bool ignore_unused_materials) {
  auto materials = json::array();
  if (ignore_unused_materials) {
    Logger::info("Ignoring unused materials");

    std::unordered_set<IdView<MaterialId>, IdViewHash<MaterialId>, std::equal_to<>> used_materials;
    find_used_materials(used_materials, tree, subtree_root_id);
    for (auto mat : used_materials) {
      json mat_json;
      serialize_material(mat_json, tree.material(mat));
      materials.push_back(mat_json);
    }
  } else {
    for (auto mat : tree.materials()) {
      json mat_json;
      serialize_material(mat_json, tree.material(mat));
      materials.push_back(mat_json);
    }
  }
  target_json["tree"]["materials"] = materials;

  auto& root_group = tree.group(subtree_root_id);
  serialize_node_common(target_json["tree"]["rootGroup"], root_group);
  auto visitor = JSONSerializerSDFTreeNodeVisitor(target_json["tree"]["rootGroup"]);
  root_group.accept_visitor(visitor);
}

void serialize_sdf_tree(json& target_json, SDFTree& tree, bool ignore_unused_materials) {
  serialize_sdf_tree(target_json, tree, tree.root().node_id(), ignore_unused_materials);
}

std::string serialize_prefab(SDFTree& tree, IdView<SDFTreeNodeId> subtree_root_id) {
  Logger::info("JSON prefab serialization started");
  try {
    json prefab_json;
    prefab_json["version"] = kNewestResinPrefabJSONSchemaVersion;
    serialize_sdf_tree(prefab_json, tree, subtree_root_id, true);

    Logger::info("JSON prefab serialization succceeded");
    return prefab_json.dump(2);
  } catch (const ResinException& e) {
    throw e;
  } catch (...) {
    log_throw(JSONSerializationException());
  }
}

void serialize_attenuation(json& target_json, const PointLight::Attenuation& attenuation) {
  target_json["constant"]  = attenuation.constant;
  target_json["linear"]    = attenuation.linear;
  target_json["quadratic"] = attenuation.quadratic;
}

void serialize_light_common(json& target_json, const BaseLightSceneComponent& light) {
  serialize_transform(target_json["transform"], light.light_base().transform);
  target_json["color"]["r"] = light.light_base().color.r;
  target_json["color"]["g"] = light.light_base().color.g;
  target_json["color"]["b"] = light.light_base().color.b;
  target_json["name"]       = light.name();
}

JSONSerializerLightSceneComponentVisitor::JSONSerializerLightSceneComponentVisitor(json& light_json)
    : json_(light_json) {}

void JSONSerializerLightSceneComponentVisitor::visit_point_light(LightSceneComponent<PointLight>& point_light) {
  serialize_light_common(json_, point_light);
  serialize_attenuation(json_["pointLight"]["attenuation"], point_light.light().attenuation);
}

void JSONSerializerLightSceneComponentVisitor::visit_directional_light(
    LightSceneComponent<DirectionalLight>& dir_light) {
  serialize_light_common(json_, dir_light);
  json_["directionalLight"]["ambientImpact"] = dir_light.light().ambient_impact;
}

std::string serialize_scene(Scene& scene) {
  Logger::info("JSON resin project serialization started");
  try {
    json scene_json;
    scene_json["version"] = kNewestResinPrefabJSONSchemaVersion;
    serialize_sdf_tree(scene_json, scene.tree(), scene.tree().root().node_id(), false);

    json lights_json = json::array();
    for (const auto& light : scene.lights()) {
      json light_json;
      auto visitor = JSONSerializerLightSceneComponentVisitor(light_json);
      light.second->accept_visitor(visitor);
      lights_json.push_back(light_json);
    }
    scene_json["lights"] = lights_json;

    Logger::info("JSON resin project serialization succceeded");
    return scene_json.dump(2);
  } catch (const ResinException& e) {
    throw e;
  } catch (...) {
    log_throw(JSONSerializationException());
  }
}

void deserialize_node_material(SDFTreeNode& node, const json& node_json,
                               const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map) {
  try {
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
  } catch (...) {
    log_throw(JSONNodeDeserializationException());
  }
}

void deserialize_node_name(SDFTreeNode& node, const json& node_json) {
  try {
    node.rename(node_json["name"]);
  } catch (...) {
    log_throw(JSONNodeDeserializationException());
  }
}

void deserialize_transform(Transform& transform, const json& trans_json) {
  try {
    transform.set_local_pos(
        glm::vec3(trans_json["position"]["x"], trans_json["position"]["y"], trans_json["position"]["z"]));
    transform.set_local_rot(glm::quat(trans_json["rotation"]["w"], trans_json["rotation"]["x"],
                                      trans_json["rotation"]["y"], trans_json["rotation"]["z"]));
    transform.set_local_scale(trans_json["scale"]);
  } catch (...) {
    log_throw(JSONTransformDeserializationException());
  }
}

void deserialize_material(MaterialSDFTreeComponent& material, const json& mat_json) {
  try {
    material.material =
        Material(glm::vec3(mat_json["albedo"]["r"], mat_json["albedo"]["g"], mat_json["albedo"]["b"]),
                 mat_json["ambient"], mat_json["diffuse"], mat_json["specular"], mat_json["specularExponent"]);
    material.rename(mat_json["name"]);
  } catch (...) {
    log_throw(JSONMaterialDeserializationException());
  }
}

void deserialize_node_bin_op(SDFTreeNode& node, const json& node_json) {
  try {
    for (auto [op, name] : kSDFBinaryOperationsJSONNames) {
      if (node_json["binaryOperation"].get<std::string>() == name) {
        node.set_bin_op(op);
        return;
      }
    }
  } catch (...) {
    log_throw(JSONNodeDeserializationException());
  }
  log_throw(JSONNodeDeserializationException(
      std::format("Provided binary operation '{}' is invalid.", node_json["binaryOperation"].get<std::string>())));
}

void deserialize_node_factor(SDFTreeNode& node, const json& node_json) {
  try {
    node.set_factor(node_json["factor"]);
  } catch (...) {
    log_throw(JSONNodeDeserializationException());
  }
}

void deserialize_node_common(SDFTreeNode& node, const json& node_json,
                             const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map) {
  deserialize_transform(node.transform(), node_json["transform"]);

  deserialize_node_material(node, node_json, material_ids_map);
  deserialize_node_name(node, node_json);
  deserialize_node_bin_op(node, node_json);
  deserialize_node_factor(node, node_json);
}

JSONDeserializerSDFTreeNodeVisitor::JSONDeserializerSDFTreeNodeVisitor(
    const json& node_json, const std::unordered_map<size_t, IdView<MaterialId>>& material_ids_map)
    : node_json_(node_json), material_ids_map_(material_ids_map) {}

void JSONDeserializerSDFTreeNodeVisitor::visit_group(GroupNode& node) {
  try {
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
  } catch (const ResinException& e) {
    Logger::warn("JSON prefab serialization failed");
    throw e;
  } catch (...) {
    Logger::warn("JSON prefab serialization failed");
    log_throw(JSONNodeDeserializationException(
        std::format("Group definition for node with name {} is invalid.", node.name())));
  }
}

void JSONDeserializerSDFTreeNodeVisitor::visit_sphere(SphereNode& node) {
  try {
    node.radius = node_json_["sphere"]["radius"];
  } catch (...) {
    log_throw(JSONNodeDeserializationException(
        std::format("Sphere definition for node with name {} is invalid.", node.name())));
  }
}

void JSONDeserializerSDFTreeNodeVisitor::visit_cube(CubeNode& node) {
  try {
    node.size.x = node_json_["cube"]["size"]["x"];
    node.size.y = node_json_["cube"]["size"]["y"];
    node.size.z = node_json_["cube"]["size"]["z"];
  } catch (...) {
    log_throw(JSONNodeDeserializationException(
        std::format("Cube definition for node with name {} is invalid.", node.name())));
  }
}

void JSONDeserializerSDFTreeNodeVisitor::visit_torus(TorusNode& node) {
  try {
    node.major_radius = node_json_["torus"]["majorRadius"];
    node.minor_radius = node_json_["torus"]["minorRadius"];
  } catch (...) {
    log_throw(JSONNodeDeserializationException(
        std::format("Torus definition for node with name {} is invalid.", node.name())));
  }
}

void JSONDeserializerSDFTreeNodeVisitor::visit_capsule(CapsuleNode& node) {
  try {
    node.height = node_json_["capsule"]["height"];
    node.radius = node_json_["capsule"]["radius"];
  } catch (...) {
    log_throw(JSONNodeDeserializationException(
        std::format("Capsule definition for node with name {} is invalid.", node.name())));
  }
}

void JSONDeserializerSDFTreeNodeVisitor::visit_link(LinkNode& node) {
  try {
    node.length       = node_json_["link"]["length"];
    node.major_radius = node_json_["link"]["majorRadius"];
    node.minor_radius = node_json_["link"]["minorRadius"];
  } catch (...) {
    log_throw(JSONNodeDeserializationException(
        std::format("Link definition for node with name {} is invalid.", node.name())));
  }
}

void JSONDeserializerSDFTreeNodeVisitor::visit_ellipsoid(EllipsoidNode& node) {
  try {
    node.radii.x = node_json_["ellipsoid"]["radii"]["x"];
    node.radii.y = node_json_["ellipsoid"]["radii"]["y"];
    node.radii.z = node_json_["ellipsoid"]["radii"]["z"];
  } catch (...) {
    log_throw(JSONNodeDeserializationException(
        std::format("Ellipsoid definition for node with name {} is invalid.", node.name())));
  }
}

void JSONDeserializerSDFTreeNodeVisitor::visit_pyramid(PyramidNode& node) {
  try {
    node.height = node_json_["pyramid"]["height"];
  } catch (...) {
    log_throw(JSONNodeDeserializationException(
        std::format("Pyramid definition for node with name {} is invalid.", node.name())));
  }
}

void JSONDeserializerSDFTreeNodeVisitor::visit_cylinder(CylinderNode& node) {
  try {
    node.height = node_json_["cylinder"]["height"];
    node.radius = node_json_["cylinder"]["radius"];
  } catch (...) {
    log_throw(JSONNodeDeserializationException(
        std::format("Cylinder definition for node with name {} is invalid.", node.name())));
  }
}

void JSONDeserializerSDFTreeNodeVisitor::visit_prism(TriangularPrismNode& node) {
  try {
    node.prismHeight = node_json_["triangularPrism"]["prismHeight"];
    node.baseHeight  = node_json_["triangularPrism"]["baseHeight"];
  } catch (...) {
    log_throw(JSONNodeDeserializationException(
        std::format("Triangular prism definition for node with name {} is invalid.", node.name())));
  }
}

void deserialize_light_common(BaseLightSceneComponent& light, const json& light_json) {
  try {
    deserialize_transform(light.light_base().transform, light_json["transform"]);
    light.light_base().color.r = light_json["color"]["r"];
    light.light_base().color.g = light_json["color"]["g"];
    light.light_base().color.b = light_json["color"]["b"];
    light.rename(light_json["name"]);
  } catch (const ResinException& e) {
    throw e;
  } catch (...) {
    log_throw(JSONLightDeserializationException());
  }
}

void deserialize_attenuation(PointLight::Attenuation& attenuation, const json& attenuation_json) {
  try {
    attenuation.constant  = attenuation_json["constant"];
    attenuation.linear    = attenuation_json["linear"];
    attenuation.quadratic = attenuation_json["quadratic"];
  } catch (...) {
    log_throw(JSONLightDeserializationException());
  }
}

JSONDeserializerLightSceneComponentVisitor::JSONDeserializerLightSceneComponentVisitor(const json& light_json)
    : json_(light_json) {}

void JSONDeserializerLightSceneComponentVisitor::visit_point_light(LightSceneComponent<PointLight>& point_light) {
  try {
    deserialize_light_common(point_light, json_);
    deserialize_attenuation(point_light.light().attenuation, json_["pointLight"]["attenuation"]);
  } catch (const ResinException& e) {
    throw e;
  } catch (...) {
    log_throw(JSONLightDeserializationException());
  }
}

void JSONDeserializerLightSceneComponentVisitor::visit_directional_light(
    LightSceneComponent<DirectionalLight>& dir_light) {
  try {
    deserialize_light_common(dir_light, json_);
    dir_light.light().ambient_impact = json_["directionalLight"]["ambientImpact"];
  } catch (const ResinException& e) {
    throw e;
  } catch (...) {
    log_throw(JSONLightDeserializationException());
  }
}

std::unique_ptr<GroupNode> deserialize_sdf_tree(SDFTree& tree, const json& tree_json) {
  std::unordered_map<size_t, IdView<MaterialId>> material_ids_map;
  for (const auto& mat_json : tree_json["materials"]) {
    auto& mat = tree.add_material(Material());
    deserialize_material(mat, mat_json);

    auto mat_it = material_ids_map.find(mat_json["id"]);
    if (mat_it != material_ids_map.end()) {
      log_throw(JSONDeserializationException(
          std::format("More than one definition of a material with id {} found.", mat_it->first)));
    }

    material_ids_map.emplace(mat_json["id"].get<size_t>(), mat.material_id());
  }

  Logger::info("Materials deserialization succeeded");

  auto root = tree.create_detached_node<GroupNode>();
  deserialize_node_common(*root, tree_json["rootGroup"], material_ids_map);
  auto visitor = JSONDeserializerSDFTreeNodeVisitor(tree_json["rootGroup"], material_ids_map);
  root->accept_visitor(visitor);

  return root;
}

std::unique_ptr<GroupNode> deserialize_prefab(SDFTree& tree, std::string_view prefab_json_str) {
  if (!json::accept(prefab_json_str)) {
    log_throw(InvalidJSONException());
  }
  Logger::info("JSON prefab deserialization started");

  try {
    auto tree_json                         = json::parse(prefab_json_str)["tree"];
    std::unique_ptr<GroupNode> prefab_root = deserialize_sdf_tree(tree, tree_json);
    Logger::info("JSON prefab deserialization succeeded");
    return prefab_root;
  } catch (const ResinException& e) {
    Logger::warn("JSON prefab deserialization failed");
    throw e;
  } catch (...) {
    Logger::warn("JSON prefab deserialization failed");
    log_throw(JSONDeserializationException());
  }
}

[[nodiscard]] std::unique_ptr<Scene> deserialize_scene(std::string_view scene_json_str) {
  if (!json::accept(scene_json_str)) {
    log_throw(InvalidJSONException());
  }
  Logger::info("JSON resin scene resin deserialization started");

  try {
    auto scene      = std::make_unique<Scene>();
    auto scene_json = json::parse(scene_json_str);

    std::unique_ptr<GroupNode> root = deserialize_sdf_tree(scene->tree(), scene_json["tree"]);
    scene->tree().set_root(std::move(root));

    if (property_exists(scene_json, "lights")) {
      for (const auto& light_json : scene_json["lights"]) {
        for (const auto& light_mapping : kLightJSONNames) {
          if (property_exists(light_json, light_mapping.second)) {
            auto visitor = JSONDeserializerLightSceneComponentVisitor(light_json);
            scene->add_light(light_mapping.first).accept_visitor(visitor);
            break;
          }
        }
      }
    }

    Logger::info("JSON resin scene deserialization succeeded");
    return scene;
  } catch (const ResinException& e) {
    Logger::warn("JSON resin scene deserialization failed");
    throw e;
  } catch (...) {
    Logger::warn("JSON resin scene deserialization failed");
    log_throw(JSONDeserializationException());
  }
}

}  // namespace json

}  // namespace resin
