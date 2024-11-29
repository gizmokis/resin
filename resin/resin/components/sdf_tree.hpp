#ifndef RESIN_TREE_COMPONENT_HPP
#define RESIN_TREE_COMPONENT_HPP

#include <imgui/imgui_internal.h>

#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf/group_node.hpp>
#include <libresin/core/sdf/sdf_tree_node.hpp>
#include <optional>

namespace resin {

class SDFTreeComponentVisitor : public IMutableSDFTreeNodeVisitor {
 public:
  void visit_sphere(SphereNode&) override {}
  void visit_cube(CubeNode&) override {}

  void visit_group(GroupNode& node) override;
  void visit_primitive(PrimitiveNode& node) override;

  inline std::optional<IdView<SDFTreeNodeId>> selected() const { return selected_; }

 private:
  std::optional<IdView<SDFTreeNodeId>> selected_ = std::nullopt;
};

namespace ImGui {  // NOLINT

std::optional<IdView<SDFTreeNodeId>> SDFTree(GroupNode& group_node);

}

}  // namespace resin

#endif
