#ifndef RESIN_PRIMITIVE_BASE_NODE_HPP
#define RESIN_PRIMITIVE_BASE_NODE_HPP

#include <libresin/core/material.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>

namespace resin {
using SDFTreePrimitiveType = sdf_shader_consts::SDFShaderPrim;
constexpr StringEnumMapping<SDFTreePrimitiveType> kSDFTreePrimitiveNames({"Sphere", "Cube"});

class BasePrimitiveNode;
using PrimitiveNodeId = Id<BasePrimitiveNode>;

class BasePrimitiveNode : public SDFTreeNode {
 public:
  constexpr static auto available_primitive_names() { return kSDFTreePrimitiveNames.names_; }

  virtual SDFTreePrimitiveType primitive_type() const = 0;
  virtual std::string_view primitive_name() const     = 0;
  virtual size_t get_component_raw_id() const         = 0;

  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override { visitor.visit_primitive(*this); }
  bool is_leaf() final { return true; }

  inline IdView<PrimitiveNodeId> primitive_id() const { return prim_id_; }

  ~BasePrimitiveNode() override = default;
  explicit BasePrimitiveNode(SDFTreeRegistry& tree, std::string_view name)
      : SDFTreeNode(tree, name), prim_id_(tree.primitives_registry), mat(glm::vec3(1.0F, 1.0F, 1.0F)) {}

  inline std::string gen_shader_code(GenShaderMode mode) const final {
    switch (mode) {
      case resin::GenShaderMode::SinglePrimitiveArray:
        return std::format(
            "{}({}[{}]*{},{}[{}])", sdf_shader_consts::kSDFShaderPrimFunctionNames.get_value(primitive_type()),
            sdf_shader_consts::kSDFShaderCoreComponentArrayNames.get_value(
                sdf_shader_consts::SDFShaderCoreComponents::Transforms),                                           //
            transform_id_.raw(),                                                                                   //
            sdf_shader_consts::kSDFShaderVariableNames.get_value(sdf_shader_consts::SDFShaderVariable::Position),  //
            sdf_shader_consts::kSDFPrimitivesArrayName,                                                            //
            prim_id_.raw()                                                                                         //
        );
      case resin::GenShaderMode::ArrayPerPrimitiveType:
        return std::format(
            "{}({}[{}]*{},{}[{}])", sdf_shader_consts::kSDFShaderPrimFunctionNames.get_value(primitive_type()),
            sdf_shader_consts::kSDFShaderCoreComponentArrayNames.get_value(
                sdf_shader_consts::SDFShaderCoreComponents::Transforms),                                           //
            transform_id_.raw(),                                                                                   //
            sdf_shader_consts::kSDFShaderVariableNames.get_value(sdf_shader_consts::SDFShaderVariable::Position),  //
            sdf_shader_consts::kSDFShaderPrimComponentArrayNames.get_value(primitive_type()),                      //
            get_component_raw_id()                                                                                 //
        );
    }

    throw NonExhaustiveEnumException();
  }

 public:
  // TODO(SDF-96): Remove and add to SDFTreeNode + handle material overriding
  Material mat;

 protected:
  inline void insert_leaves_to(
      std::unordered_set<IdView<SDFTreeNodeId>, IdViewHash<SDFTreeNodeId>, std::equal_to<>>& leaves) final {
    leaves.emplace(node_id());
  }

  inline void remove_leaves_from(
      std::unordered_set<IdView<SDFTreeNodeId>, IdViewHash<SDFTreeNodeId>, std::equal_to<>>& leaves) final {
    leaves.erase(leaves.find(node_id()));
  }

  inline void push_dirty_primitives() final { tree_registry_.dirty_primitives.emplace_back(node_id()); }

 protected:
  PrimitiveNodeId prim_id_;
};

template <SDFTreePrimitiveType PrimType>
class PrimitiveNode : public BasePrimitiveNode {
 public:
  explicit PrimitiveNode(SDFTreeRegistry& tree)
      : BasePrimitiveNode(tree, primitive_name()), comp_id_(tree.primitive_components_registry<PrimType>()) {}

  constexpr static SDFTreePrimitiveType type() { return PrimType; }
  constexpr SDFTreePrimitiveType primitive_type() const final { return PrimType; }
  constexpr std::string_view primitive_name() const final { return kSDFTreePrimitiveNames.get_value(PrimType); }

  inline IdView<Id<PrimitiveNode<PrimType>>> component_id() { return comp_id_; }

  inline size_t get_component_raw_id() const final { return comp_id_.raw(); }

 protected:
  Id<PrimitiveNode<PrimType>> comp_id_;
};

}  // namespace resin

#endif
