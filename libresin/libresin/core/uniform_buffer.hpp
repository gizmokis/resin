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

  static constexpr size_t kMaxNodeCount = 10;
  static constexpr size_t kBufferSize   = kMaxNodeCount * sizeof(Node);

  explicit UniformBuffer(size_t binding = 0);
  ~UniformBuffer();

  void bind() const;
  size_t binding() const { return binding_; }
  void unbind() const;

  void set(SDFTree& tree);
  void update_dirty(SDFTree& tree);

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
  size_t binding_;
};

}  // namespace resin

#endif  // RESIN_UNIFORM_BUFFER_HPP