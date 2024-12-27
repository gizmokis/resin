#ifndef MESH_EXPORTER_HPP
#define MESH_EXPORTER_HPP

#include <glad/gl.h>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <string>
#include <libresin/core/shader.hpp>
namespace resin {


class MeshExporter {
public:
    MeshExporter(const ComputeShaderProgram& compute_shader_program);
    ~MeshExporter();

    void export_to_obj(const std::string& output_path, const glm::vec3& grid_origin,
                       const glm::vec3& voxel_size, const unsigned int march_res);

private:
    ComputeShaderProgram compute_shader_program_;
    GLuint vertex_buffer;
    GLuint index_buffer;
    GLuint vertex_count_buffer;
    GLuint index_count_buffer;

    void initialize_buffers();
    void read_buffers(std::vector<glm::vec4>& vertices, std::vector<unsigned int>& indices) const;
    void write_obj(const std::string& output_path, const std::vector<glm::vec4>& vertices,
                   const std::vector<unsigned int>& indices);
};
}
#endif //MESH_EXPORTER_HPP
