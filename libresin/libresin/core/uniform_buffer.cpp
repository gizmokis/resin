#include <glad/gl.h>

#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/uniform_buffer.hpp>

namespace resin {

UniformBuffer::UniformBuffer(size_t binding, size_t buffer_size)
    : buffer_id_(0), binding_(binding), buffer_size_(buffer_size) {
  glGenBuffers(1, &buffer_id_);
  glBindBuffer(GL_UNIFORM_BUFFER, buffer_id_);
  glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(buffer_size_), nullptr, GL_STATIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(binding), buffer_id_);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

UniformBuffer::~UniformBuffer() { glDeleteBuffers(1, &buffer_id_); }

void UniformBuffer::bind() const { glBindBuffer(GL_UNIFORM_BUFFER, buffer_id_); }
void UniformBuffer::unbind() const { glBindBuffer(GL_UNIFORM_BUFFER, 0); }  // NOLINT

// Primitive UBO

PrimitiveUniformBuffer::PrimitiveUniformBuffer(size_t max_count)
    : UniformBuffer(0, max_count * sizeof(PrimitiveNode)), max_count_(max_count) {}

void PrimitiveUniformBuffer::set(SDFTree& tree) {  // NOLINT
  PrimitiveNodeVisitor visitor;
  tree.visit_all_primitives(visitor);
}

void PrimitiveUniformBuffer::update_dirty(SDFTree& tree) {  // NOLINT
  PrimitiveNodeVisitor visitor;
  tree.visit_dirty_primitives(visitor);
}

void PrimitiveUniformBuffer::PrimitiveNodeVisitor::visit_sphere(SphereNode& node) {
  PrimitiveNode ubo_node(node.transform(), glm::vec3(node.radius));

  glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(node.primitive_id().raw() * sizeof(PrimitiveNode)),
                  sizeof(PrimitiveNode), &ubo_node);
}

void PrimitiveUniformBuffer::PrimitiveNodeVisitor::visit_cube(CubeNode& node) {
  PrimitiveNode ubo_node(node.transform(), glm::vec3(node.size));

  glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(node.primitive_id().raw() * sizeof(PrimitiveNode)),
                  sizeof(PrimitiveNode), &ubo_node);
}

}  // namespace resin