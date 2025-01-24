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
    auto result = std::make_unique<SphereNode>(tree_registry_, radius);
    copy_common(*result, *this);
    return result;
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

  explicit CubeNode(SDFTreeRegistry& tree, glm::vec3 _size = glm::vec3(2.F));

  ~CubeNode() override = default;

  [[nodiscard]] inline std::unique_ptr<SDFTreeNode> copy() override {
    auto result = std::make_unique<CubeNode>(tree_registry_, size);
    copy_common(*result, *this);
    return result;
  }

 public:
  glm::vec3 size;
};

class TorusNode final : public PrimitiveNode<SDFTreePrimitiveType::Torus> {
 public:
  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override {
    BasePrimitiveNode::accept_visitor(visitor);
    visitor.visit_torus(*this);
  }

  explicit TorusNode(SDFTreeRegistry& tree, float _major_radius = 1.F, float _minor_radius = 0.25F);

  ~TorusNode() override = default;

  [[nodiscard]] inline std::unique_ptr<SDFTreeNode> copy() override {
    auto result = std::make_unique<TorusNode>(tree_registry_, major_radius, minor_radius);
    copy_common(*result, *this);
    return result;
  }

 public:
  float major_radius, minor_radius;
};

class CapsuleNode final : public PrimitiveNode<SDFTreePrimitiveType::Capsule> {
 public:
  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override {
    BasePrimitiveNode::accept_visitor(visitor);
    visitor.visit_capsule(*this);
  }

  explicit CapsuleNode(SDFTreeRegistry& tree, float _height = 1.F, float _radius = 0.25F);

  ~CapsuleNode() override = default;

  [[nodiscard]] inline std::unique_ptr<SDFTreeNode> copy() override {
    auto result = std::make_unique<CapsuleNode>(tree_registry_, height, radius);
    copy_common(*result, *this);
    return result;
  }

 public:
  float height, radius;
};

class LinkNode final : public PrimitiveNode<SDFTreePrimitiveType::Link> {
 public:
  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override {
    BasePrimitiveNode::accept_visitor(visitor);
    visitor.visit_link(*this);
  }

  explicit LinkNode(SDFTreeRegistry& tree, float _length = 1.F, float _major_radius = 1.F, float _minor_radius = 0.25F);

  ~LinkNode() override = default;

  [[nodiscard]] inline std::unique_ptr<SDFTreeNode> copy() override {
    auto result = std::make_unique<LinkNode>(tree_registry_, length, major_radius, minor_radius);
    copy_common(*result, *this);
    return result;
  }

 public:
  float length, major_radius, minor_radius;
};

class EllipsoidNode final : public PrimitiveNode<SDFTreePrimitiveType::Ellipsoid> {
 public:
  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override {
    BasePrimitiveNode::accept_visitor(visitor);
    visitor.visit_ellipsoid(*this);
  }

  explicit EllipsoidNode(SDFTreeRegistry& tree, glm::vec3 _radii = glm::vec3(1.F, 2.F, 3.F) / 3.F);

  ~EllipsoidNode() override = default;

  [[nodiscard]] inline std::unique_ptr<SDFTreeNode> copy() override {
    auto result = std::make_unique<EllipsoidNode>(tree_registry_, radii);
    copy_common(*result, *this);
    return result;
  }

 public:
  glm::vec3 radii;
};

class PyramidNode final : public PrimitiveNode<SDFTreePrimitiveType::Pyramid> {
 public:
  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override {
    BasePrimitiveNode::accept_visitor(visitor);
    visitor.visit_pyramid(*this);
  }

  explicit PyramidNode(SDFTreeRegistry& tree, float _height = 1.F);

  ~PyramidNode() override = default;

  [[nodiscard]] inline std::unique_ptr<SDFTreeNode> copy() override {
    auto result = std::make_unique<PyramidNode>(tree_registry_, height);
    copy_common(*result, *this);
    return result;
  }

 public:
  float height;
};

class CylinderNode final : public PrimitiveNode<SDFTreePrimitiveType::Cylinder> {
 public:
  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override {
    BasePrimitiveNode::accept_visitor(visitor);
    visitor.visit_cylinder(*this);
  }

  explicit CylinderNode(SDFTreeRegistry& tree, float _height = 1.F, float _radius = 0.25F);

  ~CylinderNode() override = default;

  [[nodiscard]] inline std::unique_ptr<SDFTreeNode> copy() override {
    auto result = std::make_unique<CylinderNode>(tree_registry_, height, radius);
    copy_common(*result, *this);
    return result;
  }

 public:
  float height, radius;
};

class TriangularPrismNode final : public PrimitiveNode<SDFTreePrimitiveType::TriangularPrism> {
 public:
  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override {
    BasePrimitiveNode::accept_visitor(visitor);
    visitor.visit_prism(*this);
  }

  explicit TriangularPrismNode(SDFTreeRegistry& tree, float _prismHeight = 1.F, float _baseHeight = 0.25F);

  ~TriangularPrismNode() override = default;

  [[nodiscard]] inline std::unique_ptr<SDFTreeNode> copy() override {
    auto result = std::make_unique<TriangularPrismNode>(tree_registry_, prismHeight, prismHeight);
    copy_common(*result, *this);
    return result;
  }

 public:
  float prismHeight, baseHeight;
};

}  // namespace resin

#endif
