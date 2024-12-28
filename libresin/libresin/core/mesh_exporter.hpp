#ifndef MESH_EXPORTER_HPP
#define MESH_EXPORTER_HPP

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glad/gl.h>

#include <assimp/Exporter.hpp>
#include <libresin/core/shader.hpp>
#include <string>
#include <vector>
namespace resin {

class MeshExporter {
 public:
  MeshExporter(const ComputeShaderProgram& compute_shader_program);
  ~MeshExporter();

  void export_to_obj(const std::string& output_path, const glm::vec3& grid_origin, const glm::vec3& voxel_size,
                     const unsigned int march_res);

 private:
  ComputeShaderProgram compute_shader_program_;
  GLuint vertex_buffer;
  GLuint vertex_count_buffer;
  GLuint normal_buffer;
  std::vector<glm::vec4> vertices_;
  std::vector<glm::vec4> normals_;

  void initialize_buffers();
  void read_buffers();
  void write_obj(const std::string& output_path);
};
}  // namespace resin
#endif  // MESH_EXPORTER_HPP
