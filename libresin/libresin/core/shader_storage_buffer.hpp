#ifndef SHADER_STORAGE_BUFFER_HPP
#define SHADER_STORAGE_BUFFER_HPP

#include <glad/gl.h>

namespace resin {
class ShaderStorageBuffer {
 public:
  ShaderStorageBuffer(size_t size, size_t binding, GLenum usage = GL_STATIC_READ);

  ~ShaderStorageBuffer() { glDeleteBuffers(1, &buffer_id_); }

  ShaderStorageBuffer(const ShaderStorageBuffer&)            = delete;
  ShaderStorageBuffer(ShaderStorageBuffer&&)                 = delete;
  ShaderStorageBuffer& operator=(const ShaderStorageBuffer&) = delete;
  ShaderStorageBuffer& operator=(ShaderStorageBuffer&&)      = delete;

  void bind() const;

  void unbind() const;

  void set_data(const void* data, size_t size) const;

  void get_data(void* output, size_t size) const;

  GLuint id() const { return buffer_id_; }

 private:
  GLuint buffer_id_;
  size_t binding_;
};
}  // namespace resin

#endif  // SHADER_STORAGE_BUFFER_HPP
