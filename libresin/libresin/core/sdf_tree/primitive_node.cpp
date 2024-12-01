
#include <libresin/core/sdf_tree/sdf_tree.hpp>

namespace resin {

void PrimitiveNode::mark_dirty() {
  if (tree_registry_.get().nodes_registry.get_max_objs() < tree_registry_.get().dirty_primitives.size()) {
    log_throw(SDFTreeReachedDirtyPrimitivesLimit());
  }

  tree_registry_.get().dirty_primitives.emplace_back(node_id());
}

SphereNode::SphereNode(SDFTreeRegistry& tree, float _radius)
    : PrimitiveNode(tree, std::format("Sphere {}", node_id_.raw())),
      radius(_radius),
      sphere_id_(tree.sphere_component_registry) {}

CubeNode::CubeNode(SDFTreeRegistry& tree, float _size)
    : PrimitiveNode(tree, std::format("Cube {}", node_id_.raw())),
      size(_size),
      cube_id_(tree.cube_component_registry) {}

}  // namespace resin
