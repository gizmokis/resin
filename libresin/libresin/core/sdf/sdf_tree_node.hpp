#ifndef RESIN_SDF_TREE_NODE_HPP
#define RESIN_SDF_TREE_NODE_HPP
#include <string>

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

class ISDFTreeNode {
 public:
  virtual std::string gen_shader_code() const                        = 0;
  virtual void accept_visitor(IMutableSDFTreeNodeVisitor& visitor)   = 0;
  virtual void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) = 0;

  virtual ~ISDFTreeNode() = default;
};

}  // namespace resin

#endif
