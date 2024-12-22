#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/json.hpp>
#include <nlohmann/json.hpp>

namespace resin {
void JSONSerializerSDFTreeNodeVisitor::visit_group(GroupNode& node) {
  json_["group"]["transform"] = transform_to_json(node.transform());
  if (node.material_id().has_value()) {
    json_["group"]["material_id"] = node.material_id()->raw();
  }
  json_["group"]["name"] = node.name();
  auto children          = json::array();
  for (auto child : node) {
    json child_json;
    JSONSerializerSDFTreeNodeVisitor visitor(child_json);
    node.get_child(child).accept_visitor(visitor);
    children.push_back(child_json);
  }
  json_["children"] = children;
}

void JSONSerializerSDFTreeNodeVisitor::visit_cube(CubeNode& node) {
  json_["cube"]["transform"] = transform_to_json(node.transform());
  json_["cube"]["name"]      = node.name();
  if (node.material_id().has_value() && !node.is_material_default()) {
    json_["cube"]["material_id"] = node.material_id()->raw();
  }
  json_["cube"]["size"]["x"] = node.size;
  json_["cube"]["size"]["y"] = node.size;
  json_["cube"]["size"]["z"] = node.size;
}

void JSONSerializerSDFTreeNodeVisitor::visit_sphere(SphereNode& node) {
  json_["sphere"]["transform"] = transform_to_json(node.transform());
  json_["sphere"]["name"]      = node.name();
  if (node.material_id().has_value() && !node.is_material_default()) {
    json_["sphere"]["material_id"] = node.material_id()->raw();
  }
  json_["sphere"]["radius"] = node.radius;
}

[[nodiscard]] json node_to_json(SDFTreeNode& node) {
  json json;
  JSONSerializerSDFTreeNodeVisitor visitor(json);
  node.accept_visitor(visitor);
  return json;
}

json transform_to_json(const Transform& transform) {
  json trans;
  trans["position"]["x"] = transform.pos().x;
  trans["position"]["y"] = transform.pos().y;
  trans["position"]["z"] = transform.pos().z;

  trans["rotation"]["x"] = transform.rot().x;
  trans["rotation"]["y"] = transform.rot().y;
  trans["rotation"]["z"] = transform.rot().z;
  trans["rotation"]["w"] = transform.rot().w;

  trans["scale"] = transform.scale();

  json json;
  json["transform"] = trans;
  return json;
}

json material_component_to_json(const MaterialSDFTreeComponent& material) {
  json mat;

  mat["id"]                = material.material_id().raw();
  mat["name"]              = material.name();
  mat["albedo"]["r"]       = material.material.albedo.r;
  mat["albedo"]["g"]       = material.material.albedo.g;
  mat["albedo"]["b"]       = material.material.albedo.b;
  mat["ambient"]           = material.material.ambientFactor;
  mat["diffuse"]           = material.material.diffuseFactor;
  mat["specular"]          = material.material.specularFactor;
  mat["specular_exponent"] = material.material.specularExponent;

  json json;
  json["material"] = mat;
  return json;
}

json sdf_tree_to_json(SDFTree& tree, IdView<SDFTreeNodeId> subtree_root_id, bool ignore_unused_materials) {
  json json;
  json["prefab"]["tree"]["root"] = node_to_json(tree.node(subtree_root_id));
  auto materials                 = json::array();

  if (ignore_unused_materials) {
    // TODO(migoox)
    throw NotImplementedException();
  } else {  // NOLINT
    for (auto mat : tree.materials()) {
      materials.push_back(material_component_to_json(tree.material(mat)));
    }
  }
  json["prefab"]["tree"]["materials"] = materials;

  return json;
}

json sdf_tree_to_json(SDFTree& tree, bool ignore_unused_materials) {
  return sdf_tree_to_json(tree, tree.root().node_id(), ignore_unused_materials);
}

}  // namespace resin
