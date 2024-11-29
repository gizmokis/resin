#ifndef RESIN_PRIMITIVE_NODE_HPP
#define RESIN_PRIMITIVE_NODE_HPP
#include <format>
#include <libresin/core/component_id_registry.hpp>
#include <libresin/core/sdf/sdf_tree_node.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/transform.hpp>

namespace resin {

class PrimitiveNode : public SDFTreeNode {
 public:
  void accept_visitor(IMutableSDFTreeNodeVisitor& visitor) override { visitor.visit_primitive(*this); }
  void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) override { visitor.visit_primitive(*this); }

  virtual ~PrimitiveNode() = default;

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
using SphereNodeId = ComponentId<SphereNode>;

class SphereNode : public PrimitiveNode {
 public:
  inline void accept_visitor(IMutableSDFTreeNodeVisitor& visitor) override {
    PrimitiveNode::accept_visitor(visitor);
    visitor.visit_sphere(*this);
  }

  inline void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) override {
    PrimitiveNode::accept_visitor(visitor);
    visitor.visit_sphere(*this);
  }

  inline std::string gen_shader_code() const override {
    return get_function_call_code(sdf_shader_consts::SDFShaderPrim::Sphere,
                                  sdf_shader_consts::SDFShaderComponents::Spheres, sphere_id_.raw());
  }

  std::string_view name() const override { return name_; }
  void rename(std::string&& name) override { name_ = std::move(name); }

  virtual ~SphereNode() = default;

  inline ComponentIdView<SphereNodeId> component_id() const { return sphere_id_; }

  explicit SphereNode(float _radius = 1.F) : radius(_radius), name_(std::format("Sphere {}", sphere_id_.raw())) {}

 public:
  float radius;

 private:
  SphereNodeId sphere_id_;
  std::string name_;
};

class CubeNode;
using CubeNodeId = ComponentId<CubeNode>;

class CubeNode : public PrimitiveNode {
 public:
  inline void accept_visitor(IMutableSDFTreeNodeVisitor& visitor) override {
    PrimitiveNode::accept_visitor(visitor);
    visitor.visit_cube(*this);
  }
  inline void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) override {
    PrimitiveNode::accept_visitor(visitor);
    visitor.visit_cube(*this);
  }

  inline std::string gen_shader_code() const override {
    return get_function_call_code(sdf_shader_consts::SDFShaderPrim::Cube, sdf_shader_consts::SDFShaderComponents::Cubes,
                                  cube_id_.raw());
  }

  std::string_view name() const override { return name_; }
  void rename(std::string&& name) override { name_ = std::move(name); }

  virtual ~CubeNode() = default;
  explicit CubeNode(float _size = 1.F) : size(_size), name_(std::format("Cube {}", cube_id_.raw())) {}

  inline ComponentIdView<CubeNodeId> component_id() const { return cube_id_; }

 public:
  float size;

 private:
  CubeNodeId cube_id_;
  std::string name_;
};

}  // namespace resin

#endif
