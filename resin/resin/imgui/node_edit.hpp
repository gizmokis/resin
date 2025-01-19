#ifndef RESIN_NODE_EDIT_HPP
#define RESIN_NODE_EDIT_HPP

#include <libresin/core/framebuffer.hpp>
#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/shader.hpp>
#include <resin/imgui/material.hpp>

namespace ImGui {  // NOLINT

namespace resin {

class SDFNodeEditVisitor : public ::resin::ISDFTreeNodeVisitor {
 public:
  void visit_sphere(::resin::SphereNode& node) override;
  void visit_cube(::resin::CubeNode& node) override;
  void visit_torus(::resin::TorusNode&) override;
  void visit_capsule(::resin::CapsuleNode&) override;
  void visit_link(::resin::LinkNode&) override;
  void visit_ellipsoid(::resin::EllipsoidNode&) override;
  void visit_pyramid(::resin::PyramidNode&) override;
  void visit_cylinder(::resin::CylinderNode&) override;
  void visit_prism(::resin::TriangularPrismNode&) override;

  void visit_group(::resin::GroupNode& node) override;

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

bool NodeEdit(::resin::SDFTreeNode& node, LazyMaterialImageFramebuffers& material_img_fbs,
              std::optional<::resin::IdView<::resin::MaterialId>>& selected_material, const ::resin::SDFTree& sdf_tree);

}  // namespace resin

}  // namespace ImGui

#endif  // RESIN_NODE_EDIT_HPP
