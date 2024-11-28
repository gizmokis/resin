#ifndef RESIN_SDF_TREE_NODE_HPP
#define RESIN_SDF_TREE_NODE_HPP
#include <string>

#include "libresin/core/id_registry.hpp"
#include "libresin/core/transform.hpp"

namespace resin {

class SphereNode;
class CubeNode;
class GroupNode;

class IMutableSDFTreeNodeVisitor {
 public:
  void virtual visit_sphere(SphereNode& node) = 0;
  void virtual visit_cube(CubeNode& node)     = 0;
  void virtual visit_group(GroupNode& node)   = 0;

  virtual ~IMutableSDFTreeNodeVisitor() = default;
};

class IImmutableSDFTreeNodeVisitor {
 public:
  void virtual visit_sphere(const SphereNode& node) = 0;
  void virtual visit_cube(const CubeNode& node)     = 0;
  void virtual visit_group(const GroupNode& node)   = 0;

  virtual ~IImmutableSDFTreeNodeVisitor() = default;
};

class SDFTreeNode {
 public:
  virtual std::string gen_shader_code() const                        = 0;
  virtual void accept_visitor(IMutableSDFTreeNodeVisitor& visitor)   = 0;
  virtual void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) = 0;

  virtual ~SDFTreeNode() = default;

  inline IdView<SDFTreeNode> get_node_id() const { return node_id_.view(); }
  inline IdView<Transform> get_transform_id() const { return transform_id_.view(); }

  inline Transform& get_transform() { return transform_; }

  // TODO(migoox): add material

 protected:
  Id<SDFTreeNode> node_id_;
  Id<Transform> transform_id_;
  Transform transform_;

  std::string unique_name_;
};

template <typename T>
concept SDFTreeNodeConcept = std::is_base_of_v<SDFTreeNode, T>;

}  // namespace resin

#endif
