#ifndef RESIN_TREE_COMPONENT_HPP
#define RESIN_TREE_COMPONENT_HPP

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <optional>

namespace ImGui {  // NOLINT

namespace resin {

class SDFTreeComponentVisitor : public ::resin::ISDFTreeNodeVisitor {
 public:
  explicit SDFTreeComponentVisitor(std::optional<::resin::IdView<::resin::SDFTreeNodeId>> selected,
                                   size_t sdf_tree_comp_id)
      : selected_(selected), payload_type_(std::format("SDF_TREE_DND_PAYLOAD_{}", sdf_tree_comp_id)) {}
  void visit_group(::resin::GroupNode& node) override;
  void visit_primitive(::resin::BasePrimitiveNode& node) override;

  void visit_root(::resin::GroupNode& node);

  inline std::optional<::resin::IdView<::resin::SDFTreeNodeId>> selected() const { return selected_; }

  void apply_move_operation(::resin::SDFTree& tree);

 private:
  void drag_and_drop(::resin::SDFTreeNode& node, bool ignore_middle);
  std::optional<::resin::IdView<::resin::SDFTreeNodeId>> get_curr_payload();

 private:
  std::optional<::resin::IdView<::resin::SDFTreeNodeId>> selected_ = std::nullopt;
  bool is_parent_selected_                                         = false;
  bool is_parent_dragged_                                          = false;

  std::optional<::resin::IdView<::resin::SDFTreeNodeId>> move_source_target_ = std::nullopt;
  std::optional<::resin::IdView<::resin::SDFTreeNodeId>> move_after_target_  = std::nullopt;
  std::optional<::resin::IdView<::resin::SDFTreeNodeId>> move_before_target_ = std::nullopt;
  std::optional<::resin::IdView<::resin::SDFTreeNodeId>> move_into_target_   = std::nullopt;

  std::string payload_type_;
};

std::optional<::resin::IdView<::resin::SDFTreeNodeId>> SDFTreeView(::resin::SDFTree& tree);

}  // namespace resin

}  // namespace ImGui

#endif
