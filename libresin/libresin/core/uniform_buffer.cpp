#include <glad/gl.h>

#include <libresin/core/uniform_buffer.hpp>

#include "libresin/core/sdf_tree/sdf_tree.hpp"

namespace resin {

UniformBuffer::UniformBuffer(size_t binding) : buffer_id_(0), binding_(binding) {
  glGenBuffers(1, &buffer_id_);
  glBindBuffer(GL_UNIFORM_BUFFER, buffer_id_);
  glBufferData(GL_UNIFORM_BUFFER, kMaxNodeCount * sizeof(Node), nullptr, GL_STATIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, binding, buffer_id_);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

UniformBuffer::~UniformBuffer() { glDeleteBuffers(1, &buffer_id_); }

void UniformBuffer::bind() const { glBindBuffer(GL_UNIFORM_BUFFER, buffer_id_); }
void UniformBuffer::unbind() const { glBindBuffer(GL_UNIFORM_BUFFER, 0); }

void UniformBuffer::set(SDFTree& tree) {
  UBONodeVisitor visitor;
  tree.visit_all_primitives(visitor);
}

void UniformBuffer::update_dirty(SDFTree& tree) {
  UBONodeVisitor visitor;
  tree.visit_dirty_primitives(visitor);
}

void UniformBuffer::UBONodeVisitor::visit_sphere(SphereNode& node) {
  Node ubo_node(node.transform(), glm::vec3(node.radius));

  glBufferSubData(GL_UNIFORM_BUFFER, node.primitive_id().raw() * sizeof(Node), sizeof(Node), &ubo_node);
}

void UniformBuffer::UBONodeVisitor::visit_cube(CubeNode& node) {
  Node ubo_node(node.transform(), glm::vec3(node.size));

  glBufferSubData(GL_UNIFORM_BUFFER, node.primitive_id().raw() * sizeof(Node), sizeof(Node), &ubo_node);
}

}  // namespace resin