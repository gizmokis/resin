#include <glad/gl.h>

#include <libresin/core/shader_storage_buffer.hpp>

namespace resin {
ShaderStorageBuffer::ShaderStorageBuffer(GLsizeiptr size, GLuint binding, GLenum usage)
    : buffer_id_(0), binding_(binding) {
  glGenBuffers(1, &buffer_id_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id_);
  glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, usage);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_, buffer_id_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
void ShaderStorageBuffer::set_data(const void* data, GLsizeiptr size) const {
  bind();
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
  unbind();
}
void ShaderStorageBuffer::get_data(void* output, GLsizeiptr size) const {
  bind();
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, output);
  unbind();
}
void ShaderStorageBuffer::bind() const { glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id_); }

void ShaderStorageBuffer::unbind() const { glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); }
}  // namespace resin
