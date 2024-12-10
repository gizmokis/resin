#ifndef RESIN_PRIMITIVE_NODE_HPP
#define RESIN_PRIMITIVE_NODE_HPP
#include <libresin/core/sdf_tree/primitive_base_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/transform.hpp>
#include <memory>

namespace resin {

class SphereNode final : public PrimitiveNode<SDFTreePrimitiveType::Sphere> {
 public:
  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override {
    BasePrimitiveNode::accept_visitor(visitor);
    visitor.visit_sphere(*this);
  }

  explicit SphereNode(SDFTreeRegistry& tree, float _radius = 1.F);

  ~SphereNode() override = default;

  [[nodiscard]] inline std::unique_ptr<SDFTreeNode> copy() override {
    return std::make_unique<SphereNode>(tree_registry_, radius);
  }

 public:
  float radius;
};

class CubeNode final : public PrimitiveNode<SDFTreePrimitiveType::Cube> {
 public:
  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override {
    BasePrimitiveNode::accept_visitor(visitor);
    visitor.visit_cube(*this);
  }

  explicit CubeNode(SDFTreeRegistry& tree, float _size = 1.F);

  ~CubeNode() override = default;

  [[nodiscard]] inline std::unique_ptr<SDFTreeNode> copy() override {
    return std::make_unique<CubeNode>(tree_registry_, size);
  }

 public:
  float size;
};

}  // namespace resin

#endif
