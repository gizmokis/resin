#ifndef RESIN_TREE_COMPONENT_HPP
#define RESIN_TREE_COMPONENT_HPP

#include <imgui/imgui_internal.h>

#include <libresin/core/sdf/group_node.hpp>
#include <libresin/core/sdf/sdf_tree_node.hpp>

namespace resin {

class SDFTreeComponentVisitor : public IMutableSDFTreeNodeVisitor {
 public:
  void visit_sphere(SphereNode&) override {}
  void visit_cube(CubeNode&) override {}

  void visit_group(GroupNode& node) override;
  void visit_primitive(PrimitiveNode& node) override;

 private:
  ImRect curr_rect_;
};

namespace ImGui {  // NOLINT

void SDFTree(GroupNode& group_node);
}

}  // namespace resin

#endif
