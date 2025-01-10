#include <glad/gl.h>

#include <cstddef>
#include <libresin/core/shader_storage_buffer.hpp>

namespace resin {
ShaderStorageBuffer::ShaderStorageBuffer(size_t size, size_t binding, GLenum usage) : buffer_id_(0), binding_(binding) {
  glGenBuffers(1, &buffer_id_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id_);
  glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(size), nullptr, usage);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, static_cast<GLuint>(binding), buffer_id_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
void ShaderStorageBuffer::set_data(const void* data, size_t size) const {
  bind();
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, static_cast<GLsizeiptr>(size), data);
  unbind();
}
void ShaderStorageBuffer::get_data(void* output, size_t size) const {
  bind();
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, static_cast<GLsizeiptr>(size), output);
  unbind();
}
void ShaderStorageBuffer::bind() const { glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id_); }

void ShaderStorageBuffer::unbind() const { glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); }  // NOLINT
}  // namespace resin
