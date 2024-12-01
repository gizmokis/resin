#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <optional>
#include <resin/components/sdf_tree.hpp>

namespace resin {

void SDFTreeComponentVisitor::visit_group(GroupNode& node) {
  level_++;
  ::ImGui::PushID(node.node_id().raw_as_int());
  bool tree_node_opened = ::ImGui::TreeNode(node.name().data());

  ::ImGui::SameLine();
  if (::ImGui::Button("Push sphere")) {
    node.push_child<SphereNode>(SDFBinaryOperation::Union);
  }
  ::ImGui::SameLine();
  if (::ImGui::Button("Push group")) {
    node.push_child<GroupNode>(SDFBinaryOperation::Union);
  }

  if (!tree_node_opened) {
    ::ImGui::PopID();
    return;
  }

  for (auto child_it = node.begin(); child_it != node.end(); ++child_it) {
    curr_children_count_ = node.get_children_count();
    node.get_child(*child_it).accept_visitor(*this);
  }

  if (node.get_children_count() == 0) {
    deleted_.push(node.node_id());
  }

  ::ImGui::TreePop();
  ::ImGui::PopID();
  level_--;
}

void SDFTreeComponentVisitor::visit_primitive(PrimitiveNode& node) {
  ::ImGui::PushID(node.node_id().raw_as_int());
  if (::ImGui::Selectable(node.name().data())) {
    selected_ = node.node_id().raw();
  }
  ::ImGui::SameLine();
  if (level_ != 1 || curr_children_count_ != 1) {
    if (::ImGui::Button("Delete")) {
      deleted_.push(node.node_id());
    }
  }

  ::ImGui::PopID();
}

void SDFTreeComponentVisitor::apply_operations(SDFTree& tree) {
  // Apply delete operations
  while (!this->deleted_.empty()) {
    auto id = this->deleted_.top();
    this->deleted_.pop();
    Logger::info("test {}", id.raw());

    if (!id.expired()) {
      tree.delete_node(id);
    }
  }
}

namespace ImGui {  // NOLINT

std::optional<size_t> SDFTreeView(SDFTree& tree) {
  static SDFTreeComponentVisitor vs;

  tree.root->accept_visitor(vs);
  vs.apply_operations(tree);

  return vs.selected();
}

}  // namespace ImGui

}  // namespace resin
