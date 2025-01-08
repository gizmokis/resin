#include <glad/gl.h>

#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/uniform_buffer.hpp>

namespace resin {

UniformBuffer::UniformBuffer(size_t max_count, size_t binding)
    : buffer_id_(0), binding_(binding), max_count_(max_count), buffer_size_(max_count * sizeof(Node)) {
  glGenBuffers(1, &buffer_id_);
  glBindBuffer(GL_UNIFORM_BUFFER, buffer_id_);
  glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(buffer_size_), nullptr, GL_STATIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(binding), buffer_id_);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

UniformBuffer::~UniformBuffer() { glDeleteBuffers(1, &buffer_id_); }

void UniformBuffer::bind() const { glBindBuffer(GL_UNIFORM_BUFFER, buffer_id_); }
void UniformBuffer::unbind() const { glBindBuffer(GL_UNIFORM_BUFFER, 0); }  // NOLINT

void UniformBuffer::set(SDFTree& tree) {  // NOLINT
  UBONodeVisitor visitor;
  tree.visit_all_primitives(visitor);
}

void UniformBuffer::set(SDFTree& tree, IdView<SDFTreeNodeId> node_id) {  // NOLINT
  UBONodeVisitor visitor;
  for (const auto prim : tree.group(node_id).primitives()) {
    tree.node(prim).accept_visitor(visitor);
  }
}

void UniformBuffer::update_dirty(SDFTree& tree) {  // NOLINT
  UBONodeVisitor visitor;
  tree.visit_dirty_primitives(visitor);
}

void UniformBuffer::UBONodeVisitor::visit_sphere(SphereNode& node) {
  Node ubo_node(node.transform(), glm::vec3(node.radius));

  glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(node.primitive_id().raw() * sizeof(Node)), sizeof(Node),
                  &ubo_node);
}

void UniformBuffer::UBONodeVisitor::visit_cube(CubeNode& node) {
  Node ubo_node(node.transform(), glm::vec3(node.size));

  glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(node.primitive_id().raw() * sizeof(Node)), sizeof(Node),
                  &ubo_node);
}

}  // namespace resin