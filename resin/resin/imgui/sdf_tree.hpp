#ifndef IMGUI_RESIN_TREE_COMPONENT_HPP
#define IMGUI_RESIN_TREE_COMPONENT_HPP

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <memory>
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
  void render_rename_popup_modal(const char* name);
  void apply_move_operation();
  void apply_duplicate_operation();

  inline std::optional<::resin::IdView<::resin::SDFTreeNodeId>> selected() const { return selected_; }

 private:
  void render_op(::resin::SDFTreeNode& node) const;
  void drag_and_drop(::resin::SDFTreeNode& node, bool ignore_middle);
  std::optional<::resin::IdView<::resin::SDFTreeNodeId>> get_curr_payload();
  std::unique_ptr<::resin::SDFTreeNode> fix_transform_and_detach(::resin::IdView<::resin::SDFTreeNodeId> source,
                                                                 ::resin::IdView<::resin::SDFTreeNodeId> new_parent);

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

  std::optional<::resin::IdView<::resin::SDFTreeNodeId>> duplicate_target_ = std::nullopt;

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

void SDFTreeView(::resin::SDFTree& tree, std::optional<::resin::IdView<::resin::SDFTreeNodeId>>& old_selected);

}  // namespace resin

}  // namespace ImGui

#endif
