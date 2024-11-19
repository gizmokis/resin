#ifndef RESIN_SHADER_HPP
#define RESIN_SHADER_HPP

#include <glad/gl.h>

#include <functional>
#include <glm/fwd.hpp>
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/utils/string_hash.hpp>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

namespace resin {
class ShaderProgram {
 public:
  explicit ShaderProgram(std::string_view name);
  virtual ~ShaderProgram() = default;

  void bind() const;
  void unbind() const;

  inline void set_bool(std::string_view name, bool value);
  inline void set_int(std::string_view name, int value);
  inline void set_int_array(std::string_view name, std::span<int> values);

  inline void set_uint(std::string_view name, uint32_t value);
  inline void set_uint_array(std::string_view name, std::span<uint32_t> values);

  inline void set_float(std::string_view name, float value);
  inline void set_float2(std::string_view name, const glm::vec2& value);
  inline void set_float3(std::string_view name, const glm::vec3& value);
  inline void set_float4(std::string_view name, const glm::vec4& value);

  inline void set_mat3(std::string_view name, const glm::mat3& value);
  inline void set_mat4(std::string_view name, const glm::mat4& value);

 protected:
  virtual void create_program() = 0;
  GLuint create_shader(const ShaderResource& resource, GLenum type);
  void link_program();

  static constexpr std::string_view get_shader_type_name(GLenum shaderType) {
    switch (shaderType) {
      case GL_VERTEX_SHADER:
        return "Vertex Shader";
      case GL_FRAGMENT_SHADER:
        return "Fragment Shader";
      case GL_COMPUTE_SHADER:
        return "Compute Shader";
      default:
        return "Unkown Shader";
    }
  }

 private:
  inline GLint get_uniform_location(std::string_view name) const;

 protected:
  std::string shader_name_;
  GLuint program_id_;

 private:
  mutable std::unordered_map<std::string, GLint, StringHash, std::equal_to<>> uniform_locations_;
};

class RenderingShaderProgram : public ShaderProgram {
 public:
  RenderingShaderProgram(std::string_view name, ShaderResource vertex_resource, ShaderResource fragment_resource);
  ~RenderingShaderProgram() override;

  const ShaderResource& vertex_shader() const { return vertex_shader_; }
  ShaderResource& vertex_shader() { return vertex_shader_; }
  const ShaderResource& fragment_shader() const { return fragment_shader_; }
  ShaderResource& fragment_shader() { return fragment_shader_; }

 private:
  void create_program() override;

 private:
  ShaderResource vertex_shader_;
  ShaderResource fragment_shader_;
};

}  // namespace resin

#endif  // RESIN_SHADER_HPP