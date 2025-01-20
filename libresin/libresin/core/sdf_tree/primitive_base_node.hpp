#ifndef RESIN_PRIMITIVE_BASE_NODE_HPP
#define RESIN_PRIMITIVE_BASE_NODE_HPP

#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <optional>

namespace resin {
class GroupNode;

using SDFTreePrimitiveType = sdf_shader_consts::SDFShaderPrim;
constexpr StringEnumMapper<SDFTreePrimitiveType> kSDFTreePrimitiveNames({
    {SDFTreePrimitiveType::Sphere, "Sphere"},                    //
    {SDFTreePrimitiveType::Cube, "Cube"},                        //
    {SDFTreePrimitiveType::Torus, "Torus"},                      //
    {SDFTreePrimitiveType::Capsule, "Capsule"},                  //
    {SDFTreePrimitiveType::Link, "Link"},                        //
    {SDFTreePrimitiveType::Ellipsoid, "Ellipsoid"},              //
    {SDFTreePrimitiveType::Pyramid, "Pyramid"},                  //
    {SDFTreePrimitiveType::Cylinder, "Cylinder"},                //
    {SDFTreePrimitiveType::TriangularPrism, "Triangular Prism"}  //
});

class BasePrimitiveNode;
using PrimitiveNodeId = Id<BasePrimitiveNode>;

class BasePrimitiveNode : public SDFTreeNode {
 public:
  constexpr static auto available_primitive_names() { return kSDFTreePrimitiveNames; }

  virtual SDFTreePrimitiveType primitive_type() const = 0;
  virtual std::string_view primitive_name() const     = 0;
  virtual size_t get_component_raw_id() const         = 0;

  inline IdView<MaterialId> default_material_id() const { return tree_registry_.default_material.material_id(); }
  std::optional<IdView<MaterialId>> active_material_id() const { return ancestor_mat_id_ ? ancestor_mat_id_ : mat_id_; }
  IdView<MaterialId> active_material_id_or_defualt() const {
    return ancestor_mat_id_ ? *ancestor_mat_id_ : mat_id_ ? *mat_id_ : default_material_id();
  }

  inline void set_material(IdView<MaterialId> mat_id) final {
    mark_dirty();
    mat_id_ = mat_id;
  }

  inline void remove_material() final {
    mark_dirty();
    mat_id_ = std::nullopt;
  }

  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override { visitor.visit_primitive(*this); }
  bool is_leaf() final { return true; }

  inline IdView<PrimitiveNodeId> primitive_id() const { return prim_id_; }

  explicit BasePrimitiveNode(SDFTreeRegistry& tree, std::string_view name)
      : SDFTreeNode(tree, name), prim_id_(tree.primitives_registry) {
    this->mark_dirty();
  }

  ~BasePrimitiveNode() override = default;

  inline std::string gen_shader_code(GenShaderMode mode) const final {
    switch (mode) {
      case resin::GenShaderMode::SinglePrimitiveArray:
        return std::format(
            "{}({},{},{})", sdf_shader_consts::kSDFShaderPrimFunctionNames[primitive_type()],
            sdf_shader_consts::kSDFShaderVariableNames[sdf_shader_consts::SDFShaderVariable::Position],  //
            node_id_.raw(),                                                                              //
            prim_id_.raw()                                                                               //
        );
      case resin::GenShaderMode::ArrayPerPrimitiveType:
        break;
    }

    throw NonExhaustiveEnumException();
  }

 protected:
  inline void insert_leaves_to(
      std::unordered_set<IdView<SDFTreeNodeId>, IdViewHash<SDFTreeNodeId>, std::equal_to<>>& leaves) final {
    leaves.emplace(node_id());
  }

  inline void remove_leaves_from(
      std::unordered_set<IdView<SDFTreeNodeId>, IdViewHash<SDFTreeNodeId>, std::equal_to<>>& leaves) final {
    leaves.erase(leaves.find(node_id()));
  }

  inline void push_dirty_primitives() final { tree_registry_.dirty_primitives.emplace(node_id()); }
  inline void set_ancestor_mat_id(IdView<MaterialId> mat_id) final { ancestor_mat_id_ = mat_id; }
  inline void remove_ancestor_mat_id() final { ancestor_mat_id_ = std::nullopt; }
  inline void delete_material_from_subtree(IdView<MaterialId> mat_id) final {
    tree_registry_.is_tree_dirty = true;
    if (mat_id == mat_id_) {
      mat_id_ = std::nullopt;
    }
  }

  void fix_material_ancestors() final;

  PrimitiveNodeId prim_id_;
};  // namespace resin

template <SDFTreePrimitiveType PrimType>
class PrimitiveNode : public BasePrimitiveNode {
 public:
  explicit PrimitiveNode(SDFTreeRegistry& tree)
      : BasePrimitiveNode(tree, primitive_name()), comp_id_(tree.primitive_components_registry<PrimType>()) {}

  constexpr static SDFTreePrimitiveType type() { return PrimType; }
  constexpr SDFTreePrimitiveType primitive_type() const final { return PrimType; }
  constexpr std::string_view primitive_name() const final { return kSDFTreePrimitiveNames[PrimType]; }

  inline IdView<Id<PrimitiveNode<PrimType>>> component_id() { return comp_id_; }

  inline size_t get_component_raw_id() const final { return comp_id_.raw(); }

 protected:
  Id<PrimitiveNode<PrimType>> comp_id_;
};

}  // namespace resin

#endif
