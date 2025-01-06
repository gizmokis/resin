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
  explicit SDFTreeComponentVisitor(::resin::SDFTree& tree,
                                   std::optional<::resin::IdView<::resin::SDFTreeNodeId>> selected)
      : selected_(selected), payload_type_(std::format("SDF_TREE_DND_PAYLOAD_{}", tree.tree_id())), sdf_tree_(tree) {}
  void visit_group(::resin::GroupNode& node) override;
  void visit_primitive(::resin::BasePrimitiveNode& node) override;

  void render_tree();
  void apply_move_operation();

  inline std::optional<::resin::IdView<::resin::SDFTreeNodeId>> selected() const { return selected_; }

 private:
  void render_op(::resin::SDFTreeNode& node) const;
  void drag_and_drop(::resin::SDFTreeNode& node, bool ignore_middle);
  std::optional<::resin::IdView<::resin::SDFTreeNodeId>> get_curr_payload();

 private:
  std::optional<::resin::IdView<::resin::SDFTreeNodeId>> selected_ = std::nullopt;
  bool is_parent_selected_                                         = false;
  bool is_parent_dragged_                                          = false;
  bool is_first_                                                   = false;
  bool is_any_node_clicked_                                        = false;

  std::optional<::resin::IdView<::resin::SDFTreeNodeId>> move_source_target_ = std::nullopt;
  std::optional<::resin::IdView<::resin::SDFTreeNodeId>> move_after_target_  = std::nullopt;
  std::optional<::resin::IdView<::resin::SDFTreeNodeId>> move_before_target_ = std::nullopt;
  std::optional<::resin::IdView<::resin::SDFTreeNodeId>> move_into_target_   = std::nullopt;

  std::string payload_type_;
  static constexpr ::resin::StringEnumMapper<::resin::SDFBinaryOperation> kOperationSymbol =
      ::resin::StringEnumMapper<::resin::SDFBinaryOperation>({
          {::resin::SDFBinaryOperation::Union, "+"},         //
          {::resin::SDFBinaryOperation::SmoothUnion, "+'"},  //
          {::resin::SDFBinaryOperation::Diff, "-"},          //
          {::resin::SDFBinaryOperation::SmoothDiff, "-'"},   //
          {::resin::SDFBinaryOperation::Inter, "&"},         //
          {::resin::SDFBinaryOperation::SmoothInter, "&'"},  //
          {::resin::SDFBinaryOperation::Xor, "^"},           //
          {::resin::SDFBinaryOperation::SmoothXor, "^'"}     //
      });

  ::resin::SDFTree& sdf_tree_;  // NOLINT
};

std::optional<::resin::IdView<::resin::SDFTreeNodeId>> SDFTreeView(
    ::resin::SDFTree& tree, const std::optional<::resin::IdView<::resin::SDFTreeNodeId>>& old_selected);

}  // namespace resin

}  // namespace ImGui

#endif
