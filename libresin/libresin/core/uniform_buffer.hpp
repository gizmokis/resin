#ifndef RESIN_UNIFORM_BUFFER_HPP
#define RESIN_UNIFORM_BUFFER_HPP

#include <glad/gl.h>

#include <libresin/core/material.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <libresin/core/transform.hpp>

namespace resin {

class UniformBuffer {
 public:
  explicit UniformBuffer(size_t binding, size_t buffer_size);
  virtual ~UniformBuffer();

  void bind() const;
  void unbind() const;

  size_t binding() const { return binding_; }
  size_t buffer_size() const { return buffer_size_; }

  UniformBuffer(const UniformBuffer&)            = delete;
  UniformBuffer(UniformBuffer&&)                 = delete;
  UniformBuffer& operator=(const UniformBuffer&) = delete;
  UniformBuffer& operator=(UniformBuffer&&)      = delete;

 private:
  GLuint buffer_id_;
  const size_t binding_, buffer_size_;
};

class PrimitiveUniformBuffer : public UniformBuffer {
 public:
  struct PrimitiveNode {
    glm::mat4 transform;
    glm::vec3 size;
    float scale;

    PrimitiveNode(const Transform& _transform, const glm::vec3& _size)
        : transform(_transform.world_to_local_matrix()), size(_size), scale(_transform.scale()) {}
  };

  explicit PrimitiveUniformBuffer(size_t max_count);
  ~PrimitiveUniformBuffer() override = default;

  size_t max_count() const { return max_count_; }

  void set(SDFTree& tree);
  void update_dirty(SDFTree& tree);

  PrimitiveUniformBuffer(const PrimitiveUniformBuffer&)            = delete;
  PrimitiveUniformBuffer(PrimitiveUniformBuffer&&)                 = delete;
  PrimitiveUniformBuffer& operator=(const PrimitiveUniformBuffer&) = delete;
  PrimitiveUniformBuffer& operator=(PrimitiveUniformBuffer&&)      = delete;

 private:
  class PrimitiveNodeVisitor : public ISDFTreeNodeVisitor {
    void visit_sphere(SphereNode& node) override;
    void visit_cube(CubeNode& node) override;
  };

  const size_t max_count_;
};

}  // namespace resin

#endif  // RESIN_UNIFORM_BUFFER_HPP