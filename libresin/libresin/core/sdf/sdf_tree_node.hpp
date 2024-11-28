#ifndef RESIN_SDF_TREE_NODE_HPP
#define RESIN_SDF_TREE_NODE_HPP
#include <libresin/core/id_registry.hpp>
#include <libresin/core/transform.hpp>
#include <string>

namespace resin {

class SphereNode;
class CubeNode;
class GroupNode;
class PrimitiveNode;

class IMutableSDFTreeNodeVisitor {
 public:
  void virtual visit_sphere(SphereNode&)       = 0;
  void virtual visit_cube(CubeNode&)           = 0;
  void virtual visit_group(GroupNode&)         = 0;
  void virtual visit_primitive(PrimitiveNode&) = 0;

  virtual ~IMutableSDFTreeNodeVisitor() = default;
};

class IImmutableSDFTreeNodeVisitor {
 public:
  void virtual visit_sphere(const SphereNode&)       = 0;
  void virtual visit_cube(const CubeNode&)           = 0;
  void virtual visit_group(const GroupNode&)         = 0;
  void virtual visit_primitive(const PrimitiveNode&) = 0;

  virtual ~IImmutableSDFTreeNodeVisitor() = default;
};

class SDFTreeNode {
 public:
  virtual std::string gen_shader_code() const                        = 0;
  virtual void accept_visitor(IMutableSDFTreeNodeVisitor& visitor)   = 0;
  virtual void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) = 0;
  virtual std::string_view name() const                              = 0;
  virtual void rename(std::string&&)                                 = 0;

  virtual ~SDFTreeNode() = default;

  inline IdView<SDFTreeNode> node_id() const { return node_id_.view(); }
  inline IdView<Transform> transform_id() const { return transform_id_.view(); }
  inline Transform& transform() { return transform_; }

  // TODO(migoox): add material

 protected:
  Id<SDFTreeNode> node_id_;
  Id<Transform> transform_id_;
  Transform transform_;
};

template <typename T>
concept SDFTreeNodeConcept = std::is_base_of_v<SDFTreeNode, T>;

}  // namespace resin

#endif
