#ifndef RESIN_SHADER_HPP
#define RESIN_SHADER_HPP

#include <glad/gl.h>

#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
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
  virtual ~ShaderProgram();

  void bind() const;
  void unbind() const;

  void recompile();

  template <typename T>
  inline void set_uniform(std::string_view name, const T& value) {
    GLint location = get_uniform_location(name);
    if constexpr (std::is_same_v<T, bool>) {
      glProgramUniform1i(program_id_, location, value ? 1 : 0);
    } else if constexpr (std::is_same_v<T, int>) {
      glProgramUniform1i(program_id_, location, value);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
      glProgramUniform1ui(program_id_, location, value);
    } else if constexpr (std::is_same_v<T, float>) {
      glProgramUniform1f(program_id_, location, value);
    } else if constexpr (std::is_same_v<T, glm::vec2>) {
      glProgramUniform2f(program_id_, location, value.x, value.y);
    } else if constexpr (std::is_same_v<T, glm::vec3>) {
      glProgramUniform3f(program_id_, location, value.x, value.y, value.z);
    } else if constexpr (std::is_same_v<T, glm::vec4>) {
      glProgramUniform4f(program_id_, location, value.x, value.y, value.z, value.w);
    } else if constexpr (std::is_same_v<T, glm::mat3>) {
      glProgramUniformMatrix3fv(program_id_, location, 1, GL_FALSE, glm::value_ptr(value));
    } else if constexpr (std::is_same_v<T, glm::mat4>) {
      glProgramUniformMatrix4fv(program_id_, location, 1, GL_FALSE, glm::value_ptr(value));
    } else {
      static_assert(false, "Unsupported uniform type");
    }
  }

  template <typename T>
  inline void set_uniform_array(std::string_view name, std::span<T> values) {
    GLint location = get_uniform_location(name);
    if constexpr (std::is_same_v<T, int>) {
      glProgramUniform1iv(program_id_, get_uniform_location(name), static_cast<GLsizei>(values.size()), values.data());
    } else if constexpr (std::is_same_v<T, uint32_t>) {
      glProgramUniform1uiv(program_id_, get_uniform_location(name), static_cast<GLsizei>(values.size()), values.data());
    } else if constexpr (std::is_same_v<T, glm::vec3>) {
      glProgramUniform3fv(program_id_, get_uniform_location(name), static_cast<GLsizei>(values.size()), glm::value_ptr(values[0]));
    } else {
      static_assert(false, "Unsupported uniform array type");
    }
  }

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
  GLint get_uniform_location(std::string_view name) const;

 protected:
  std::string shader_name_;
  GLuint program_id_;

 private:
  mutable std::unordered_map<std::string, GLint, StringHash, std::equal_to<>> uniform_locations_;
};

class RenderingShaderProgram : public ShaderProgram {
 public:
  RenderingShaderProgram(std::string_view name, ShaderResource vertex_resource, ShaderResource fragment_resource);

  const ShaderResource& vertex_shader() const { return vertex_shader_; }
  ShaderResource& vertex_shader() { return vertex_shader_; }
  const ShaderResource& fragment_shader() const { return fragment_shader_; }
  ShaderResource& fragment_shader() { return fragment_shader_; }

  GLuint id() const {return program_id_;}

 private:
  void create_program() override;

 private:
  ShaderResource vertex_shader_;
  ShaderResource fragment_shader_;
};

}  // namespace resin

#endif  // RESIN_SHADER_HPP