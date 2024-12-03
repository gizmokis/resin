#ifndef RESIN_TREE_COMPONENT_HPP
#define RESIN_TREE_COMPONENT_HPP

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <optional>

namespace resin {

class SDFTreeComponentVisitor : public ISDFTreeNodeVisitor {
 public:
  explicit SDFTreeComponentVisitor(std::optional<IdView<SDFTreeNodeId>> selected) : selected_(selected) {}
  void visit_group(GroupNode& node) override;
  void visit_primitive(PrimitiveNode& node) override;

  inline std::optional<IdView<SDFTreeNodeId>> selected() const { return selected_; }

 private:
  std::optional<IdView<SDFTreeNodeId>> selected_ = std::nullopt;
  bool is_parent_selected_                       = false;
};

class SDFTreeOperationVisitor : public ISDFTreeNodeVisitor {
 public:
  void visit_group(GroupNode& node) override;
  void visit_primitive(PrimitiveNode& node) override;

  enum class Operation {
    PushPrimitive,
    PushGroup,
  };
  Operation op;
};

namespace ImGui {  // NOLINT

std::optional<IdView<SDFTreeNodeId>> SDFTreeView(SDFTree& tree);

}

}  // namespace resin

#endif
