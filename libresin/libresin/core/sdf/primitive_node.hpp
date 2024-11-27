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
  Transform transform_;

 protected:
  inline std::string get_function_call_code(sdf_shader_consts::SDFShaderPrim primitive,
                                            sdf_shader_consts::SDFShaderComponents component,
                                            size_t component_id) const {
    return std::format(
        "{}({}[{}]*{}, {}[{}])", sdf_shader_consts::kSDFShaderPrimFunctionNames.get_name(primitive),
        sdf_shader_consts::kSDFShaderComponentArrayNames.get_name(
            sdf_shader_consts::SDFShaderComponents::Transforms),                                              //
        transform_id_.get_raw(),                                                                              //
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
                                  sdf_shader_consts::SDFShaderComponents::Spheres, sphere_id_.get_raw());
  }

  inline TypedId<SphereNode> get_component_id() const { return sphere_id_; }

  explicit SphereNode(float radius = 1.F)
      : radius(radius), sphere_id_(IdRegistry<SphereNode>::get_instance().register_id()) {}

 public:
  float radius;

 private:
  TypedId<SphereNode> sphere_id_;
};

class CubeNode : public PrimitiveNode {
 public:
  inline void accept_visitor(IMutableSDFTreeNodeVisitor& visitor) override { visitor.visit_cube(*this); }
  inline void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) override { visitor.visit_cube(*this); }

  inline std::string gen_shader_code() const override {
    return get_function_call_code(sdf_shader_consts::SDFShaderPrim::Cube, sdf_shader_consts::SDFShaderComponents::Cubes,
                                  cube_id_.get_raw());
  }

  explicit CubeNode(float size = 1.F) : size(size), cube_id_(IdRegistry<CubeNode>::get_instance().register_id()) {}

  inline TypedId<CubeNode> get_component_id() const { return cube_id_; }

 public:
  float size;

 private:
  TypedId<CubeNode> cube_id_;
};

}  // namespace resin

#endif
