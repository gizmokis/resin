
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>

namespace resin {

SphereNode::SphereNode(SDFTreeRegistry& tree, float _radius)
    : PrimitiveNode<SDFTreePrimitiveType::Sphere>(tree), radius(_radius) {}

CubeNode::CubeNode(SDFTreeRegistry& tree, float _size) : PrimitiveNode<SDFTreePrimitiveType::Cube>(tree), size(_size) {}

}  // namespace resin
