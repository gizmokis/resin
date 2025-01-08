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
  struct Node {
    glm::mat4 transform;
    glm::vec3 size;
    float scale;

    Node(const Transform& _transform, const glm::vec3& _size)
        : transform(_transform.world_to_local_matrix()), size(_size), scale(_transform.scale()) {}
  };

  explicit UniformBuffer(size_t max_count, size_t binding = 0);
  ~UniformBuffer();

  void bind() const;
  size_t binding() const { return binding_; }
  void unbind() const;

  void set(SDFTree& tree);
  void set(SDFTree& tree, IdView<SDFTreeNodeId> node_id);
  void update_dirty(SDFTree& tree);

  size_t max_count() const { return max_count_; }
  size_t buffer_size() const { return buffer_size_; }

  UniformBuffer(const UniformBuffer&)            = delete;
  UniformBuffer(UniformBuffer&&)                 = delete;
  UniformBuffer& operator=(const UniformBuffer&) = delete;
  UniformBuffer& operator=(UniformBuffer&&)      = delete;

 private:
  class UBONodeVisitor : public ISDFTreeNodeVisitor {
    void visit_sphere(SphereNode& node) override;
    void visit_cube(CubeNode& node) override;
  };

  GLuint buffer_id_;
  const size_t binding_;
  const size_t max_count_, buffer_size_;
};

}  // namespace resin

#endif  // RESIN_UNIFORM_BUFFER_HPP