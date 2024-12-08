#ifndef RESIN_TREE_COMPONENT_HPP
#define RESIN_TREE_COMPONENT_HPP

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <optional>

namespace ImGui {  // NOLINT

namespace resin {

class SDFTreeComponentVisitor : public ::resin::ISDFTreeNodeVisitor {
 public:
  explicit SDFTreeComponentVisitor(std::optional<::resin::IdView<::resin::SDFTreeNodeId>> selected)
      : selected_(selected) {}
  void visit_group(::resin::GroupNode& node) override;
  void visit_primitive(::resin::PrimitiveNode& node) override;

  inline std::optional<::resin::IdView<::resin::SDFTreeNodeId>> selected() const { return selected_; }

 private:
  std::optional<::resin::IdView<::resin::SDFTreeNodeId>> selected_ = std::nullopt;
  bool is_parent_selected_                                         = false;
};

class SDFTreeOperationVisitor : public ::resin::ISDFTreeNodeVisitor {
 public:
  void visit_group(::resin::GroupNode& node) override;
  void visit_primitive(::resin::PrimitiveNode& node) override;

  enum class Operation {
    PushPrimitive,
    PushGroup,
  };
  Operation op;
};

std::optional<::resin::IdView<::resin::SDFTreeNodeId>> SDFTreeView(::resin::SDFTree& tree);

}  // namespace resin

}  // namespace ImGui

#endif
