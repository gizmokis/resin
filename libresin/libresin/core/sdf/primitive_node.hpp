#ifndef RESIN_PRIMITIVE_NODE_HPP
#define RESIN_PRIMITIVE_NODE_HPP
#include <format>
#include <libresin/core/sdf/sdf_tree_node.hpp>
#include <libresin/core/sdf_shader_consts.hpp>

namespace resin {

class IPrimitiveNode : public ISDFTreeNode {
 public:
  std::string gen_shader_code() const override;

  void accept_visitor(IMutableSDFTreeNodeVisitor& visitor) override;
  void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) override;
  ~IPrimitiveNode() override = default;

 protected:
  inline static std::string get_function_call_code(sdf_shader_consts::SDFShaderPrim prim,
                                                   sdf_shader_consts::SDFShaderComponents component,
                                                   size_t transform_id, size_t component_id) {
    return std::format(
        "{}({}[{}]*{}, {}[{}])", sdf_shader_consts::kSDFShaderPrimFunctionNames.get_name(prim),
        sdf_shader_consts::kSDFShaderComponentArrayNames.get_name(sdf_shader_consts::SDFShaderComponents::Transforms),
        transform_id,
        sdf_shader_consts::kSDFShaderVariableNames.get_name(sdf_shader_consts::SDFShaderVariable::Position),
        sdf_shader_consts::kSDFShaderComponentArrayNames.get_name(component), component_id);
  }
};

class SphereNode : public IPrimitiveNode {
 public:
  inline void accept_visitor(IMutableSDFTreeNodeVisitor& visitor) override { visitor.visit_sphere(*this); }
  inline void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) override { visitor.visit_sphere(*this); }

  inline std::string gen_shader_code() const override {
    return get_function_call_code(sdf_shader_consts::SDFShaderPrim::Sphere,
                                  sdf_shader_consts::SDFShaderComponents::Spheres, 0, 0);
  }
};

class CubeNode : public IPrimitiveNode {
 public:
  inline void accept_visitor(IMutableSDFTreeNodeVisitor& visitor) override { visitor.visit_cube(*this); }
  inline void accept_visitor(IImmutableSDFTreeNodeVisitor& visitor) override { visitor.visit_cube(*this); }
  inline std::string gen_shader_code() const override {
    return get_function_call_code(sdf_shader_consts::SDFShaderPrim::Cube, sdf_shader_consts::SDFShaderComponents::Cubes,
                                  0, 0);
  }
};

}  // namespace resin

#endif
