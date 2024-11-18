#ifndef RESIN_SHADER_HPP
#define RESIN_SHADER_HPP

#include <glad/gl.h>

#include <functional>
#include <glm/fwd.hpp>
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

  void set_bool(std::string_view name, bool value);
  void set_int(std::string_view name, int value);
  void set_int_array(std::string_view name, std::span<int> values);

  void set_uint(std::string_view name, uint32_t value);
  void set_uint_array(std::string_view name, std::span<uint32_t> values);

  void set_float(std::string_view name, float value);
  void set_float2(std::string_view name, const glm::vec2& value);
  void set_float3(std::string_view name, const glm::vec3& value);
  void set_float4(std::string_view name, const glm::vec4& value);

  void set_mat3(std::string_view name, const glm::mat3& value);
  void set_mat4(std::string_view name, const glm::mat4& value);

 private:
  inline GLint get_uniform_location(std::string_view name) const;

 private:
  std::string shader_name_;
  GLuint renderer_id_ = 0;

  mutable std::unordered_map<std::string, GLint, StringHash, std::equal_to<>> uniform_locations_;
};

class RenderingShaderProgram : public ShaderProgram {
 public:
};

}  // namespace resin

#endif  // RESIN_SHADER_HPP