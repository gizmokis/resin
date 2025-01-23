#include <glad/gl.h>

#include <libresin/core/material.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/uniform_buffer.hpp>

namespace resin {

UniformBuffer::UniformBuffer(size_t binding, size_t item_count, size_t item_size, size_t item_end_padding)
    : buffer_id_(0), binding_(binding), buffer_size_(item_size * item_count), item_end_padding_(item_end_padding) {
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
    : UniformBuffer(0, max_count, sizeof(PrimitiveNode), 12), max_count_(max_count) {}

void PrimitiveUniformBuffer::set(SDFTree& tree) {  // NOLINT
  PrimitiveNodeVisitor visitor;
  tree.visit_all_primitives(visitor);
}

void PrimitiveUniformBuffer::update_dirty(SDFTree& tree) {  // NOLINT
  PrimitiveNodeVisitor visitor;
  tree.visit_dirty_primitives(visitor);
}

void PrimitiveUniformBuffer::PrimitiveNodeVisitor::visit_sphere(SphereNode& node) {
  PrimitiveNode ubo_node(node, glm::vec3(node.radius));

  glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(node.primitive_id().raw() * sizeof(PrimitiveNode)),
                  sizeof(PrimitiveNode), &ubo_node);
}

void PrimitiveUniformBuffer::PrimitiveNodeVisitor::visit_cube(CubeNode& node) {
  PrimitiveNode ubo_node(node, glm::vec3(node.size));

  glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(node.primitive_id().raw() * sizeof(PrimitiveNode)),
                  sizeof(PrimitiveNode), &ubo_node);
}

void PrimitiveUniformBuffer::PrimitiveNodeVisitor::visit_torus(TorusNode& node) {
  PrimitiveNode ubo_node(node, glm::vec3(node.major_radius, node.minor_radius, 0));

  glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(node.primitive_id().raw() * sizeof(PrimitiveNode)),
                  sizeof(PrimitiveNode), &ubo_node);
}

void PrimitiveUniformBuffer::PrimitiveNodeVisitor::visit_capsule(CapsuleNode& node) {
  PrimitiveNode ubo_node(node, glm::vec3(node.height, node.radius, 0));

  glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(node.primitive_id().raw() * sizeof(PrimitiveNode)),
                  sizeof(PrimitiveNode), &ubo_node);
}

void PrimitiveUniformBuffer::PrimitiveNodeVisitor::visit_link(LinkNode& node) {
  PrimitiveNode ubo_node(node, glm::vec3(node.length, node.major_radius, node.minor_radius));

  glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(node.primitive_id().raw() * sizeof(PrimitiveNode)),
                  sizeof(PrimitiveNode), &ubo_node);
}

void PrimitiveUniformBuffer::PrimitiveNodeVisitor::visit_ellipsoid(EllipsoidNode& node) {
  PrimitiveNode ubo_node(node, node.radii);

  glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(node.primitive_id().raw() * sizeof(PrimitiveNode)),
                  sizeof(PrimitiveNode), &ubo_node);
}

void PrimitiveUniformBuffer::PrimitiveNodeVisitor::visit_pyramid(PyramidNode& node) {
  PrimitiveNode ubo_node(node, glm::vec3(node.height, 0, 0));

  glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(node.primitive_id().raw() * sizeof(PrimitiveNode)),
                  sizeof(PrimitiveNode), &ubo_node);
}

void PrimitiveUniformBuffer::PrimitiveNodeVisitor::visit_cylinder(CylinderNode& node) {
  PrimitiveNode ubo_node(node, glm::vec3(node.height, node.radius, 0));

  glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(node.primitive_id().raw() * sizeof(PrimitiveNode)),
                  sizeof(PrimitiveNode), &ubo_node);
}

void PrimitiveUniformBuffer::PrimitiveNodeVisitor::visit_prism(TriangularPrismNode& node) {
  PrimitiveNode ubo_node(node, glm::vec3(node.prismHeight, node.baseHeight, 0));

  glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(node.primitive_id().raw() * sizeof(PrimitiveNode)),
                  sizeof(PrimitiveNode), &ubo_node);
}

MaterialUniformBuffer::MaterialUniformBuffer(size_t max_count)
    : UniformBuffer(1, sizeof(Material), max_count, 4), max_count_(max_count) {}

void MaterialUniformBuffer::set(SDFTree& tree) {  // NOLINT
  tree.visit_all_materials([](auto& mat) {
    glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(mat.material_id().raw() * sizeof(Material)),
                    sizeof(Material), &mat.material);
  });
}

void MaterialUniformBuffer::update_dirty(SDFTree& tree) {  // NOLINT
  tree.visit_dirty_materials([](auto& mat) {
    glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(mat.material_id().raw() * sizeof(Material)),
                    sizeof(Material), &mat.material);
  });
}

}  // namespace resin
