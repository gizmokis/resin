#ifndef RESIN_PRIMITIVE_NODE_HPP
#define RESIN_PRIMITIVE_NODE_HPP
#include <format>
#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf/sdf_tree_node.hpp>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/transform.hpp>

namespace resin {

class PrimitiveNode : public SDFTreeNode {
 public:
  std::string gen_shader_code() const override = 0;

  void accept_visitor(IMutableSDFTreeNodeVisitor& visitor) override   = 0;
  void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) override = 0;

  ~PrimitiveNode() override = default;

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

class SphereNode : public PrimitiveNode {
 public:
  inline void accept_visitor(IMutableSDFTreeNodeVisitor& visitor) override { visitor.visit_sphere(*this); }
  inline void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) override { visitor.visit_sphere(*this); }

  inline std::string gen_shader_code() const override {
    return get_function_call_code(sdf_shader_consts::SDFShaderPrim::Sphere,
                                  sdf_shader_consts::SDFShaderComponents::Spheres, sphere_id_.raw());
  }

  inline IdView<SphereNode> get_component_id() const { return sphere_id_.view(); }

  explicit SphereNode(float radius = 1.F) : radius(radius) {}

 public:
  float radius;

 private:
  Id<SphereNode> sphere_id_;
};

class CubeNode : public PrimitiveNode {
 public:
  inline void accept_visitor(IMutableSDFTreeNodeVisitor& visitor) override { visitor.visit_cube(*this); }
  inline void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) override { visitor.visit_cube(*this); }

  inline std::string gen_shader_code() const override {
    return get_function_call_code(sdf_shader_consts::SDFShaderPrim::Cube, sdf_shader_consts::SDFShaderComponents::Cubes,
                                  cube_id_.raw());
  }

  explicit CubeNode(float size = 1.F) : size(size) {}

  inline IdView<CubeNode> get_component_id() const { return cube_id_.view(); }

 public:
  float size;

 private:
  Id<CubeNode> cube_id_;
};

}  // namespace resin

#endif
