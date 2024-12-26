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
    MeshExporter(const std::string& compute_shader_path);
    ~MeshExporter();

    void export_to_obj(const std::string& output_path, const glm::vec3& grid_origin,
                       const glm::vec3& voxel_size, const glm::ivec3& grid_resolution);

private:
    ShaderResourceManager shader_resource_manager_;
    ComputeShaderProgram compute_shader_program;
    GLuint vertex_buffer;
    GLuint index_buffer;
    GLuint vertex_count_buffer;
    GLuint index_count_buffer;

    void initialize_buffers();
    void read_buffers(std::vector<glm::vec3>& vertices, std::vector<unsigned int>& indices);
    void write_obj(const std::string& output_path, const std::vector<glm::vec3>& vertices,
                   const std::vector<unsigned int>& indices);
};
}
#endif //MESH_EXPORTER_HPP
