
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>

namespace resin {

SphereNode::SphereNode(SDFTreeRegistry& tree, float _radius)
    : PrimitiveNode<SDFTreePrimitiveType::Sphere>(tree), radius(_radius) {}

CubeNode::CubeNode(SDFTreeRegistry& tree, glm::vec3 _size)
    : PrimitiveNode<SDFTreePrimitiveType::Cube>(tree), size(std::move(_size)) {}

TorusNode::TorusNode(SDFTreeRegistry& tree, float _major_radius, float _minor_radius)
    : PrimitiveNode<SDFTreePrimitiveType::Torus>(tree), major_radius(_major_radius), minor_radius(_minor_radius) {}

CapsuleNode::CapsuleNode(SDFTreeRegistry& tree, float _height, float _radius)
    : PrimitiveNode<SDFTreePrimitiveType::Capsule>(tree), height(_height), radius(_radius) {}

LinkNode::LinkNode(SDFTreeRegistry& tree, float _length, float _major_radius, float _minor_radius)
    : PrimitiveNode<SDFTreePrimitiveType::Link>(tree),
      length(_length),
      major_radius(_major_radius),
      minor_radius(_minor_radius) {}

EllipsoidNode::EllipsoidNode(SDFTreeRegistry& tree, glm::vec3 _radii)
    : PrimitiveNode<SDFTreePrimitiveType::Ellipsoid>(tree), radii(std::move(_radii)) {}

PyramidNode::PyramidNode(SDFTreeRegistry& tree, float _height)
    : PrimitiveNode<SDFTreePrimitiveType::Pyramid>(tree), height(_height) {}

CylinderNode::CylinderNode(SDFTreeRegistry& tree, float _height, float _radius)
    : PrimitiveNode<SDFTreePrimitiveType::Cylinder>(tree), height(_height), radius(_radius) {}

TriangularPrismNode::TriangularPrismNode(SDFTreeRegistry& tree, float _prismHeight, float _baseHeight)
    : PrimitiveNode<SDFTreePrimitiveType::TriangularPrism>(tree), prismHeight(_prismHeight), baseHeight(_baseHeight) {}

}  // namespace resin
