#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_base_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>
#include <optional>
#include <string>
#include <utility>

namespace resin {

GroupNode::GroupNode(SDFTreeRegistry& tree) : SDFTreeNode(tree, "Group") {
  tree_registry_.all_group_nodes[node_id_.raw()] = *this;
  mark_primitives_dirty();
  mark_dirty();
}
GroupNode::~GroupNode() { tree_registry_.all_group_nodes[node_id_.raw()] = std::nullopt; }

SDFTreeNode& GroupNode::push_back_primitive(SDFTreePrimitiveType type, SDFBinaryOperation bin_op) {
  switch (type) {
    case SDFTreePrimitiveType::Sphere:
      return push_back_child<SphereNode>(bin_op);
    case SDFTreePrimitiveType::Cube:
      return push_back_child<CubeNode>(bin_op);
    case SDFTreePrimitiveType::Torus:
      return push_back_child<TorusNode>(bin_op);
    case SDFTreePrimitiveType::Capsule:
      return push_back_child<CapsuleNode>(bin_op);
    case SDFTreePrimitiveType::Link:
      return push_back_child<LinkNode>(bin_op);
    case SDFTreePrimitiveType::Ellipsoid:
      return push_back_child<EllipsoidNode>(bin_op);
    case SDFTreePrimitiveType::Pyramid:
      return push_back_child<PyramidNode>(bin_op);
    case SDFTreePrimitiveType::Cylinder:
      return push_back_child<CylinderNode>(bin_op);
    case SDFTreePrimitiveType::TriangularPrism:
      return push_back_child<TriangularPrismNode>(bin_op);
    case resin::SDFTreePrimitiveType::_Count:
      throw NonExhaustiveEnumException();
  }

  throw NonExhaustiveEnumException();
}

bool GroupNode::is_node_shallow(IdView<SDFTreeNodeId> id) const {
  return tree_registry_.all_group_nodes[id.raw()].has_value() &&
         tree_registry_.all_group_nodes[id.raw()].value().get().primitives().size() == 0;
}

std::string GroupNode::gen_shader_code(GenShaderMode mode) const {
  size_t non_shallow_nodes_count = 0;
  auto first_non_shallow_node    = nodes_order_.begin();
  auto second_non_shallow_node   = nodes_order_.begin();
  auto last_non_shallow_node     = nodes_order_.begin();
  for (auto it = nodes_order_.begin(); it != nodes_order_.end(); ++it) {
    if (!is_node_shallow(*it)) {
      if (non_shallow_nodes_count == 0) {
        first_non_shallow_node = it;
      }
      if (non_shallow_nodes_count == 1) {
        second_non_shallow_node = it;
      }
      last_non_shallow_node = it;
      non_shallow_nodes_count++;
    }
  }

  if (non_shallow_nodes_count == 0) {
    // The current node must be a root
    return "";
  }

  if (non_shallow_nodes_count == 1) {
    // If there is one node only, the operation is ignored
    return get_child(*last_non_shallow_node).gen_shader_code(mode);
  }

  std::string sdf;
  sdf += sdf_shader_consts::kSDFScaleFunctionName;
  sdf += "(";
  for (auto it = last_non_shallow_node; it != first_non_shallow_node; --it) {
    if (is_node_shallow(*it)) {
      continue;
    }

    sdf += sdf_shader_consts::kSDFShaderBinOpFunctionNames[get_child(*it).bin_op()];
    sdf += "(";
  }
  sdf += get_child(*first_non_shallow_node).gen_shader_code(mode);
  sdf += ",";

  for (auto it = second_non_shallow_node; it != nodes_order_.end(); ++it) {
    if (is_node_shallow(*it)) {
      continue;
    }

    auto& child = get_child(*it);
    sdf += child.gen_shader_code(mode);
    if (child.has_smooth_bin_op()) {
      sdf += ",";
      sdf += std::to_string(child.node_id().raw());
    }
    sdf += "),";
  }
  sdf += std::to_string(node_id_.raw());
  sdf += ")";
  return sdf;
}

void GroupNode::set_as_parent_of(std::unique_ptr<SDFTreeNode>& node_ptr) {
  Logger::info("Setting new parent with id {} for node with id {}", node_id().raw(), node_ptr->node_id().raw());
  node_ptr->set_parent(*this);
  node_ptr->transform().set_parent(transform_);
  if (mat_id_) {
    node_ptr->set_ancestor_mat_id(*mat_id_);
    node_ptr->mark_primitives_dirty();
  } else if (ancestor_mat_id_) {
    node_ptr->set_ancestor_mat_id(*ancestor_mat_id_);
    node_ptr->mark_primitives_dirty();
  }
  insert_leaves_up(node_ptr);
}

void GroupNode::remove_from_parent_of(std::unique_ptr<SDFTreeNode>& node_ptr) {
  node_ptr->remove_parent();
  node_ptr->transform().remove_parent();
  if (mat_id_ || ancestor_mat_id_) {
    node_ptr->remove_ancestor_mat_id();
    node_ptr->mark_primitives_dirty();
  }
  remove_leaves_up(node_ptr);
}

SDFTreeNode& GroupNode::get_child(IdView<SDFTreeNodeId> node_id) const {
  if (!tree_registry_.all_nodes[node_id.raw()].has_value()) {
    log_throw(SDFTreeNodeDoesNotExist(node_id.raw()));
  }

  if (!is_child(node_id)) {
    log_throw(SDFTreeNodeIsNotAChild(node_id.raw(), node_id_.raw()));
  }

  return tree_registry_.all_nodes[node_id.raw()]->get();
}

void GroupNode::set_ancestor_mat_id(IdView<MaterialId> mat_id) {
  ancestor_mat_id_ = mat_id;

  if (!mat_id_.has_value()) {
    for (auto& it : *this) {
      get_child(it).set_ancestor_mat_id(mat_id);
    }
  }
}

void GroupNode::remove_ancestor_mat_id() {
  ancestor_mat_id_ = std::nullopt;

  if (!mat_id_.has_value()) {
    for (auto& it : *this) {
      get_child(it).remove_ancestor_mat_id();
    }
  }
}

void GroupNode::fix_material_ancestors() {
  mark_primitives_dirty();

  if (!parent_.has_value()) {
    ancestor_mat_id_ = std::nullopt;
  } else {
    ancestor_mat_id_ = parent_->get().active_material_id();
  }

  for (auto& it : *this) {
    get_child(it).fix_material_ancestors();
  }
}

void GroupNode::set_material(IdView<MaterialId> mat_id) {
  mark_primitives_dirty();
  mat_id_ = mat_id;

  for (auto& it : *this) {
    get_child(it).set_ancestor_mat_id(mat_id);
  }
}

void GroupNode::remove_material() {
  mark_primitives_dirty();
  mat_id_ = std::nullopt;

  if (ancestor_mat_id_) {
    for (auto& it : *this) {
      get_child(it).set_ancestor_mat_id(*ancestor_mat_id_);
    }
  } else {
    for (auto& it : *this) {
      get_child(it).remove_ancestor_mat_id();
    }
  }
}

void GroupNode::delete_material_from_subtree(IdView<MaterialId> mat_id) {
  mark_primitives_dirty();
  if (mat_id == mat_id_) {
    mat_id_ = std::nullopt;
  }

  for (auto it : *this) {
    get_child(it).delete_material_from_subtree(mat_id);
  }
}

void GroupNode::delete_child(IdView<SDFTreeNodeId> node_id) {
  auto it = nodes_.find(node_id);
  if (it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild(node_id.raw(), node_id_.raw()));
  }

  tree_registry_.is_tree_dirty = true;
  remove_from_parent_of(it->second.second);

  nodes_order_.erase(it->second.first);
  nodes_.erase(it);
}

bool GroupNode::child_has_neighbor_prev(IdView<SDFTreeNodeId> node_id) const {
  auto map_it = nodes_.find(node_id);
  if (map_it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild(node_id.raw(), node_id_.raw()));
  }

  return map_it->second.first != nodes_order_.begin();
}

bool GroupNode::child_has_neighbor_next(IdView<SDFTreeNodeId> node_id) const {
  auto map_it = nodes_.find(node_id);
  if (map_it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild(node_id.raw(), node_id_.raw()));
  }

  auto it = map_it->second.first;
  return ++it != nodes_order_.end();
}

SDFTreeNode& GroupNode::child_neighbor_prev(IdView<SDFTreeNodeId> node_id) {
  auto map_it = nodes_.find(node_id);
  if (map_it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild(node_id.raw(), node_id_.raw()));
  }

  auto list_it = map_it->second.first;
  if (list_it == nodes_order_.begin()) {
    log_throw(SDFTreeNodeIsNotAChild(node_id.raw(), node_id_.raw()));
  }

  return *nodes_.find(*(++list_it))->second.second;
}

SDFTreeNode& GroupNode::child_neighbor_next(IdView<SDFTreeNodeId> node_id) {
  auto map_it = nodes_.find(node_id);
  if (map_it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild(node_id.raw(), node_id_.raw()));
  }

  auto list_it = map_it->second.first;
  if (++list_it == nodes_order_.end()) {
    log_throw(SDFTreeNodeIsNotAChild(node_id.raw(), node_id_.raw()));
  }

  return *nodes_.find(*list_it)->second.second;
}

std::unique_ptr<SDFTreeNode> GroupNode::detach_child(IdView<SDFTreeNodeId> node_id) {
  tree_registry_.is_tree_dirty = true;
  auto map_it                  = nodes_.find(node_id);
  if (map_it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild(node_id.raw(), node_id_.raw()));
  }

  nodes_order_.erase(map_it->second.first);
  auto child_ptr = std::move(map_it->second.second);
  nodes_.erase(map_it);

  remove_from_parent_of(child_ptr);

  return child_ptr;
}

void GroupNode::push_back_child(std::unique_ptr<SDFTreeNode> node_ptr) {
  tree_registry_.is_tree_dirty = true;
  set_as_parent_of(node_ptr);
  auto node_id = node_ptr->node_id();

  nodes_order_.push_back(node_id);
  nodes_.emplace(node_id, std::make_pair(std::prev(nodes_order_.end()), std::move(node_ptr)));
}

void GroupNode::push_front_child(std::unique_ptr<SDFTreeNode> node_ptr) {
  tree_registry_.is_tree_dirty = true;
  set_as_parent_of(node_ptr);
  auto node_id = node_ptr->node_id();

  nodes_order_.push_front(node_id);
  nodes_.emplace(node_id, std::make_pair(nodes_order_.begin(), std::move(node_ptr)));
}

void GroupNode::insert_before_child(std::optional<IdView<SDFTreeNodeId>> before_child_id,
                                    std::unique_ptr<SDFTreeNode> node_ptr) {
  if (!before_child_id.has_value()) {
    push_back_child(std::move(node_ptr));
    return;
  }
  tree_registry_.is_tree_dirty = true;
  set_as_parent_of(node_ptr);

  auto map_it = nodes_.find(*before_child_id);
  if (map_it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild(before_child_id->raw(), node_id_.raw()));
  }
  auto pos = map_it->second.first;

  auto list_it = nodes_order_.emplace(pos, node_ptr->node_id());
  auto node_id = node_ptr->node_id();
  nodes_.emplace(node_id, std::make_pair(list_it, std::move(node_ptr)));
}

void GroupNode::insert_after_child(std::optional<IdView<SDFTreeNodeId>> after_child_id,
                                   std::unique_ptr<SDFTreeNode> node_ptr) {
  if (!after_child_id.has_value()) {
    push_front_child(std::move(node_ptr));
    return;
  }
  tree_registry_.is_tree_dirty = true;
  set_as_parent_of(node_ptr);

  auto map_it = nodes_.find(*after_child_id);
  if (map_it == nodes_.end()) {
    log_throw(SDFTreeNodeIsNotAChild(after_child_id->raw(), node_id_.raw()));
  }
  auto pos = map_it->second.first;

  auto list_it = nodes_order_.emplace(++pos, node_ptr->node_id());
  auto node_id = node_ptr->node_id();
  nodes_.emplace(node_id, std::make_pair(list_it, std::move(node_ptr)));
}

void GroupNode::push_dirty_primitives() {
  for (auto prim : leaves_) {
    tree_registry_.dirty_primitives.emplace(std::move(prim));
  }
}

std::unique_ptr<SDFTreeNode> GroupNode::copy() {
  auto result = std::make_unique<GroupNode>(tree_registry_);
  copy_common(*result, *this);
  for (auto& list_it : nodes_order_) {
    result->push_back_child(nodes_.find(list_it)->second.second->copy());
  }

  return result;
}

void GroupNode::insert_leaves_up(const std::unique_ptr<SDFTreeNode>& source) {
  source->insert_leaves_to(leaves_);
  if (parent_.has_value()) {
    parent_->get().insert_leaves_up(source);
  }
}

void GroupNode::remove_leaves_up(const std::unique_ptr<SDFTreeNode>& source) {
  source->remove_leaves_from(leaves_);
  if (parent_.has_value()) {
    parent_->get().remove_leaves_up(source);
  }
}

}  // namespace resin
