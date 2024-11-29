#ifndef RESIN_SDF_TREE_NODE_HPP
#define RESIN_SDF_TREE_NODE_HPP
#include <libresin/core/id_registry.hpp>
#include <libresin/core/transform.hpp>
#include <string>

namespace resin {
class IMutableSDFTreeNodeVisitor;
class IImmutableSDFTreeNodeVisitor;

class SDFTreeNode;
using SDFTreeNodeId = Id<SDFTreeNode, 10000>;  // NOLINT

class SDFTreeNode {
 public:
  virtual std::string gen_shader_code() const                        = 0;
  virtual void accept_visitor(IMutableSDFTreeNodeVisitor& visitor)   = 0;
  virtual void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) = 0;
  virtual std::string_view name() const                              = 0;
  virtual void rename(std::string&&)                                 = 0;

  virtual ~SDFTreeNode() = default;

  inline IdView<SDFTreeNodeId> node_id() const { return IdView<SDFTreeNodeId>(node_id_); }
  inline IdView<TransformId> transform_id() const { return IdView<TransformId>(transform_id_); }
  inline Transform& transform() { return transform_; }

  // TODO(migoox): add material

 protected:
  SDFTreeNodeId node_id_;
  TransformId transform_id_;
  Transform transform_;
};

template <typename T>
concept SDFTreeNodeConcept = std::is_base_of_v<SDFTreeNode, T>;

class GroupNode;
class PrimitiveNode;
class SphereNode;
class CubeNode;

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
}  // namespace resin

#endif
