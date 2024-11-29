
#include <libresin/core/sdf_tree/sdf_tree.hpp>

namespace resin {

SphereNode::SphereNode(SDFTreeRegistry& tree, float _radius)
    : PrimitiveNode(tree),
      radius(_radius),
      sphere_id_(tree.sphere_component_registry),
      name_(std::format("Sphere {}", node_id_.raw())) {}

CubeNode::CubeNode(SDFTreeRegistry& tree, float _size)
    : PrimitiveNode(tree),
      size(_size),
      cube_id_(tree.cube_component_registry),
      name_(std::format("Cube {}", node_id_.raw())) {}

}  // namespace resin
