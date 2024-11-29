#ifndef RESIN_SDF_TREE_NODE_HPP
#define RESIN_SDF_TREE_NODE_HPP
#include <libresin/core/component_id_registry.hpp>
#include <libresin/core/transform.hpp>
#include <memory>
#include <string>

namespace resin {

class SDFTreeNode;
class GroupNode;
class PrimitiveNode;
class SphereNode;
class CubeNode;

class SDFTreeRegistry {
 public:
  // TODO(migoox): allow specifying the sizes
  SDFTreeRegistry()
      : sphere_component_registry_(std::make_shared<IdRegistry<SphereNode>>(1000)),
        cube_component_registry_(std::make_shared<IdRegistry<CubeNode>>(1000)),
        transform_component_registry_(std::make_shared<IdRegistry<Transform>>(2000)),
        nodes_registry_(std::make_shared<IdRegistry<SDFTreeNode>>(5000)) {
    Logger::debug("registry constructor");
  }

 private:
  friend SDFTreeNode;
  friend SphereNode;
  friend CubeNode;

  // Components: the ids correspond to the uniform buffers indices
  std::shared_ptr<IdRegistry<SphereNode>> sphere_component_registry_;
  std::shared_ptr<IdRegistry<CubeNode>> cube_component_registry_;
  std::shared_ptr<IdRegistry<Transform>> transform_component_registry_;

  // Nodes: the ids correspond to the nodes stored in the array of all nodes
  std::shared_ptr<IdRegistry<SDFTreeNode>> nodes_registry_;
};

class SDFTree {
 public:
  SDFTree()
      : sdf_tree_registry_(std::make_shared<SDFTreeRegistry>()),
        root(std::make_shared<GroupNode>(sdf_tree_registry_)) {}

 private:
  std::shared_ptr<SDFTreeRegistry> sdf_tree_registry_;

 public:
  std::shared_ptr<GroupNode> root;
};

class ISDFTreeNodeVisitor {
 public:
  void virtual visit_sphere(SphereNode&)       = 0;
  void virtual visit_cube(CubeNode&)           = 0;
  void virtual visit_group(GroupNode&)         = 0;
  void virtual visit_primitive(PrimitiveNode&) = 0;

  virtual ~ISDFTreeNodeVisitor() = default;
};

using SDFTreeNodeId = Id<SDFTreeNode>;

class SDFTreeNode {
 public:
  virtual std::string gen_shader_code() const               = 0;
  virtual void accept_visitor(ISDFTreeNodeVisitor& visitor) = 0;
  virtual std::string_view name() const                     = 0;
  virtual void rename(std::string&&)                        = 0;

  virtual ~SDFTreeNode() = default;

  SDFTreeNode() = delete;
  explicit SDFTreeNode(const std::shared_ptr<SDFTreeRegistry>& tree)
      : node_id_(tree->nodes_registry_), transform_id_(tree->transform_component_registry_), tree_(tree) {}

  inline IdView<SDFTreeNodeId> node_id() const { return node_id_; }
  inline IdView<TransformId> transform_component_id() const { return transform_id_; }
  inline Transform& transform() { return transform_; }

  // TODO(migoox): add material component

 protected:
  SDFTreeNodeId node_id_;
  TransformId transform_id_;
  Transform transform_;
  std::shared_ptr<SDFTreeRegistry> tree_;
};

template <typename T>
concept SDFTreeNodeConcept = std::is_base_of_v<SDFTreeNode, T>;

}  // namespace resin

#endif
