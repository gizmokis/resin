#ifndef NODE_EDIT_HPP
#define NODE_EDIT_HPP

#include <libresin/core/sdf_tree/sdf_tree_node.hpp>

namespace ImGui {  // NOLINT

namespace resin {

class SDFNodeEditVisitor : public ::resin::ISDFTreeNodeVisitor {
 public:
  void visit_sphere(::resin::SphereNode& node) override;
  void visit_cube(::resin::CubeNode& node) override;

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
};

bool NodeEdit(::resin::SDFTreeNode& node);

}  // namespace resin

}  // namespace ImGui

#endif  // NODE_EDIT_HPP
