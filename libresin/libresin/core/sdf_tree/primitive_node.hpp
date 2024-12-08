#ifndef RESIN_PRIMITIVE_NODE_HPP
#define RESIN_PRIMITIVE_NODE_HPP
#include <format>
#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/transform.hpp>
#include <memory>
#include <unordered_set>

namespace resin {

using SDFTreeShaderComponent = sdf_shader_consts::SDFShaderComponents;
using SDFTreePrimitive       = sdf_shader_consts::SDFShaderPrim;
constexpr EnumMapping<SDFTreePrimitive> kSDFTreePrimitiveNames({"Sphere", "Cube"});

class PrimitiveNode : public SDFTreeNode {
 public:
  constexpr static auto primitive_names() { return kSDFTreePrimitiveNames.names_; }

  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override { visitor.visit_primitive(*this); }
  bool is_leaf() final { return true; }

  virtual SDFTreePrimitive primitive_type() const       = 0;
  virtual SDFTreeShaderComponent component_type() const = 0;

  std::string_view primitive_name() const { return kSDFTreePrimitiveNames.get_name(primitive_type()); }

  ~PrimitiveNode() override = default;
  explicit PrimitiveNode(SDFTreeRegistry& tree, std::string_view name) : SDFTreeNode(tree, name) {}

  inline std::string gen_shader_code() const final {
    return std::format(
        "{}({}[{}]*{},{}[{}])", sdf_shader_consts::kSDFShaderPrimFunctionNames.get_name(primitive_type()),
        sdf_shader_consts::kSDFShaderComponentArrayNames.get_name(
            sdf_shader_consts::SDFShaderComponents::Transforms),                                              //
        transform_id_.raw(),                                                                                  //
        sdf_shader_consts::kSDFShaderVariableNames.get_name(sdf_shader_consts::SDFShaderVariable::Position),  //
        sdf_shader_consts::kSDFShaderComponentArrayNames.get_name(component_type()),                          //
        get_component_raw_id()                                                                                //
    );
  }

 protected:
  virtual size_t get_component_raw_id() const = 0;

  inline void insert_leaves_to(
      std::unordered_set<IdView<SDFTreeNodeId>, IdViewHash<SDFTreeNodeId>, std::equal_to<>>& leaves) final {
    leaves.emplace(node_id());
  }

  inline void remove_leaves_from(
      std::unordered_set<IdView<SDFTreeNodeId>, IdViewHash<SDFTreeNodeId>, std::equal_to<>>& leaves) final {
    leaves.erase(leaves.find(node_id()));
  }

  void push_dirty_primitives() final;
};

class SphereNode;
using SphereNodeId = Id<SphereNode>;

class SphereNode final : public PrimitiveNode {
 public:
  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override {
    PrimitiveNode::accept_visitor(visitor);
    visitor.visit_sphere(*this);
  }

  explicit SphereNode(SDFTreeRegistry& tree, float _radius = 1.F);

  inline SDFTreePrimitive primitive_type() const override { return SDFTreePrimitive::Sphere; }
  inline SDFTreeShaderComponent component_type() const override { return SDFTreeShaderComponent::Spheres; }

  ~SphereNode() override = default;
  inline IdView<SphereNodeId> component_id() const { return sphere_id_; }

  [[nodiscard]] inline std::unique_ptr<SDFTreeNode> copy() override {
    return std::make_unique<SphereNode>(tree_registry_, radius);
  }

 protected:
  inline size_t get_component_raw_id() const override { return sphere_id_.raw(); }

 public:
  float radius;

 private:
  SphereNodeId sphere_id_;
};

class CubeNode;
using CubeNodeId = Id<CubeNode>;

class CubeNode final : public PrimitiveNode {
 public:
  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override {
    PrimitiveNode::accept_visitor(visitor);
    visitor.visit_cube(*this);
  }

  explicit CubeNode(SDFTreeRegistry& tree, float _size = 1.F);

  inline SDFTreePrimitive primitive_type() const override { return SDFTreePrimitive::Cube; }
  inline SDFTreeShaderComponent component_type() const override { return SDFTreeShaderComponent::Cubes; }

  ~CubeNode() override = default;
  inline IdView<CubeNodeId> component_id() const { return cube_id_; }

  [[nodiscard]] inline std::unique_ptr<SDFTreeNode> copy() override {
    return std::make_unique<CubeNode>(tree_registry_, size);
  }

 protected:
  inline size_t get_component_raw_id() const override { return cube_id_.raw(); }

 public:
  float size;

 private:
  CubeNodeId cube_id_;
};

}  // namespace resin

#endif
