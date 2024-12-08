
#include <libresin/core/sdf_tree/sdf_tree.hpp>

namespace resin {

void PrimitiveNode::push_dirty_primitives() { tree_registry_.dirty_primitives.emplace_back(node_id()); }

SphereNode::SphereNode(SDFTreeRegistry& tree, float _radius)
    : PrimitiveNode(tree, "Sphere"), radius(_radius), sphere_id_(tree.sphere_component_registry) {}

CubeNode::CubeNode(SDFTreeRegistry& tree, float _size)
    : PrimitiveNode(tree, "Cube"), size(_size), cube_id_(tree.cube_component_registry) {}

}  // namespace resin
