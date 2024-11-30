#ifndef RESIN_PRIMITIVE_NODE_HPP
#define RESIN_PRIMITIVE_NODE_HPP
#include <format>
#include <functional>
#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/transform.hpp>

namespace resin {

class PrimitiveNode : public SDFTreeNode {
 public:
  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override { visitor.visit_primitive(*this); }

  virtual ~PrimitiveNode() = default;
  explicit PrimitiveNode(SDFTreeRegistry& tree) : SDFTreeNode(tree) {}

 protected:
  inline std::string get_function_call_code(sdf_shader_consts::SDFShaderPrim primitive,
                                            sdf_shader_consts::SDFShaderComponents component,
                                            size_t component_id) const {
    return std::format(
        "{}({}[{}]*{}, {}[{}])", sdf_shader_consts::kSDFShaderPrimFunctionNames.get_name(primitive),
        sdf_shader_consts::kSDFShaderComponentArrayNames.get_name(
            sdf_shader_consts::SDFShaderComponents::Transforms),                                              //
        transform_id_.raw(),                                                                                  //
        sdf_shader_consts::kSDFShaderVariableNames.get_name(sdf_shader_consts::SDFShaderVariable::Position),  //
        sdf_shader_consts::kSDFShaderComponentArrayNames.get_name(component),                                 //
        component_id                                                                                          //
    );
  }
};

class SphereNode;
using SphereNodeId = Id<SphereNode>;

class SphereNode : public PrimitiveNode {
 public:
  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override {
    PrimitiveNode::accept_visitor(visitor);
    visitor.visit_sphere(*this);
  }

  inline std::string gen_shader_code() const override {
    return get_function_call_code(sdf_shader_consts::SDFShaderPrim::Sphere,
                                  sdf_shader_consts::SDFShaderComponents::Spheres, sphere_id_.raw());
  }

  inline std::string_view name() const override { return name_; }
  inline void rename(std::string&& name) override { name_ = std::move(name); }

  virtual ~SphereNode() = default;

  inline IdView<SphereNodeId> component_id() const { return sphere_id_; }

  explicit SphereNode(SDFTreeRegistry& tree, float _radius = 1.F);

 public:
  float radius;

 private:
  SphereNodeId sphere_id_;
  std::string name_;
};

class CubeNode;
using CubeNodeId = Id<CubeNode>;

class CubeNode : public PrimitiveNode {
 public:
  inline void accept_visitor(ISDFTreeNodeVisitor& visitor) override {
    PrimitiveNode::accept_visitor(visitor);
    visitor.visit_cube(*this);
  }

  inline std::string gen_shader_code() const override {
    return get_function_call_code(sdf_shader_consts::SDFShaderPrim::Cube, sdf_shader_consts::SDFShaderComponents::Cubes,
                                  cube_id_.raw());
  }

  inline std::string_view name() const override { return name_; }
  inline void rename(std::string&& name) override { name_ = std::move(name); }

  virtual ~CubeNode() = default;
  explicit CubeNode(SDFTreeRegistry& tree, float _size = 1.F);
  inline IdView<CubeNodeId> component_id() const { return cube_id_; }

 public:
  float size;

 private:
  CubeNodeId cube_id_;
  std::string name_;
};

}  // namespace resin

#endif
