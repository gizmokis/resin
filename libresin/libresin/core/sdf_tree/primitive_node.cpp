
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>

namespace resin {

SphereNode::SphereNode(SDFTreeRegistry& tree, float _radius)
    : PrimitiveNode<SDFTreePrimitiveType::Sphere>(tree), radius(_radius), mat(glm::vec3(1.0F, 1.0F, 0.0F)) {}

CubeNode::CubeNode(SDFTreeRegistry& tree, float _size)
    : PrimitiveNode<SDFTreePrimitiveType::Cube>(tree), size(_size), mat(glm::vec3(0.0F, 1.0F, 1.0F)) {}

}  // namespace resin
