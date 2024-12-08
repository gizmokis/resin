#ifndef RESIN_PRIMITIVE_BASE_NODE_HPP
#define RESIN_PRIMITIVE_BASE_NODE_HPP

#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>

namespace resin {
using SDFTreePrimitiveType = sdf_shader_consts::SDFShaderPrim;
constexpr StringEnumMapping<SDFTreePrimitiveType> kSDFTreePrimitiveNames({"Sphere", "Cube"});

class BasePrimitiveNode : public SDFTreeNode {
 public:
  constexpr static auto available_primitive_names() { return kSDFTreePrimitiveNames.names_; }

  virtual SDFTreePrimitiveType primitive_type() const = 0;
  constexpr std::string_view primitive_name() const { return kSDFTreePrimitiveNames.get_value(primitive_type()); }
  virtual size_t get_component_raw_id() const = 0;

  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override { visitor.visit_primitive(*this); }
  bool is_leaf() final { return true; }

  ~BasePrimitiveNode() override = default;
  explicit BasePrimitiveNode(SDFTreeRegistry& tree, std::string_view name) : SDFTreeNode(tree, name) {}

  inline std::string gen_shader_code() const final {
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
};

template <SDFTreePrimitiveType PrimType>
class PrimitiveNode : public BasePrimitiveNode {
 public:
  explicit PrimitiveNode(SDFTreeRegistry& tree)
      : BasePrimitiveNode(tree, primitive_name()), comp_id_(tree.primitve_components_registry<PrimType>()) {}

  constexpr static SDFTreePrimitiveType type() { return PrimType; }
  constexpr SDFTreePrimitiveType primitive_type() const final { return PrimType; }

  inline IdView<Id<PrimitiveNode<PrimType>>> component_id() { return comp_id_; }

  inline size_t get_component_raw_id() const final { return comp_id_.raw(); }

 protected:
  Id<PrimitiveNode<PrimType>> comp_id_;
};

}  // namespace resin

#endif
