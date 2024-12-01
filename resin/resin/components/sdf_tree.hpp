#ifndef RESIN_TREE_COMPONENT_HPP
#define RESIN_TREE_COMPONENT_HPP

#include <imgui/imgui_internal.h>

#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <optional>

namespace resin {

class SDFTreeComponentVisitor : public ISDFTreeNodeVisitor {
 public:
  void visit_sphere(SphereNode&) override {}
  void visit_cube(CubeNode&) override {}

  void visit_group(GroupNode& node) override;
  void visit_primitive(PrimitiveNode& node) override;

  inline std::optional<size_t> selected() const { return selected_; }

  void apply_operations(SDFTree& tree);
  void reset();

 private:
  std::optional<size_t> selected_ = std::nullopt;

  std::stack<IdView<SDFTreeNodeId>> deleted_;

  int level_                  = 0;
  size_t curr_children_count_ = 0;
};

namespace ImGui {  // NOLINT

std::optional<size_t> SDFTreeView(SDFTree& tree);

}

}  // namespace resin

#endif
