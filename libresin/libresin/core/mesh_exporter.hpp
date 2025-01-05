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

  void export_mesh(const std::string& output_path, const std::string& format, const glm::vec3& bb_start, const glm::vec3& bb_end,
    const unsigned int march_res);

 private:
  ComputeShaderProgram compute_shader_program_;
  GLuint vertex_buffer;
  GLuint vertex_count_buffer;
  GLuint normal_buffer;
  GLuint uv_buffer;
  GLuint output_texture;

  std::vector<glm::vec4> vertices_;
  std::vector<glm::vec4> normals_;
  std::vector<glm::vec2> uvs_;

  aiScene* scene_;


  void initialize_buffers(unsigned int march_res);
  void execute_shader(const glm::vec3 bb_start, const glm::vec3 bb_end, const unsigned int march_res) const;
  void read_buffers();
  void create_scene();
  void export_scene(const std::string& output_path, const std::string& format) const;
};
}  // namespace resin
#endif  // MESH_EXPORTER_HPP
