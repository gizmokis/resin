#include <assimp/scene.h>
#include <glad/gl.h>

#include <assimp/Exporter.hpp>
#include <libresin/core/shader.hpp>
#include <string>
#include <vector>
#include <libresin/core/mesh_exporter.hpp>

namespace resin {
class MeshExporter {
 public:
  MeshExporter(const std::string& compute_shader_path)
      : compute_shader_program_("MarchingCubesComputeShader", ShaderResource(compute_shader_path, ShaderType::Compute)) {
    initializeBuffers();
  }

  ~MeshExporter() {
    glDeleteBuffers(1, &vertex_buffer_);
    glDeleteBuffers(1, &index_buffer_);
  }

  void exportToObj(const std::string& outputPath, const glm::vec3& gridOrigin, const glm::vec3& voxelSize,
                   const glm::ivec3& gridResolution) {
    // Set up compute shader.
    computeShaderProgram.bind();
    computeShaderProgram.setUniform("gridOrigin", gridOrigin);
    computeShaderProgram.setUniform("voxelSize", voxelSize);
    computeShaderProgram.setUniform("gridResolution", gridResolution);

    // Dispatch compute shader.
    glDispatchCompute(gridResolution.x / 8, gridResolution.y / 8, gridResolution.z / 8);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Read back data.
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
    readBuffers(vertices, indices);

    // Export mesh.
    write_obj(outputPath, vertices, indices);
  }

 private:
  ComputeShaderProgram compute_shader_program_;
  GLuint vertex_buffer_      = 0;
  GLuint index_buffer_       = 0;
  GLuint vertex_count_buffer_ = 0;
  GLuint index_count_buffer_  = 0;

  void initializeBuffers() {
    // Create buffers for vertices and indices.
    glGenBuffers(1, &vertex_buffer_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_buffer_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3) * 1000000, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_buffer_);

    glGenBuffers(1, &index_buffer_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, index_buffer_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * 1000000, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, index_buffer_);

    // Create buffers for vertex and index counts.
    glGenBuffers(1, &vertex_count_buffer_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_count_buffer_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vertex_count_buffer_);

    glGenBuffers(1, &index_count_buffer_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, index_count_buffer_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, index_count_buffer_);
  }

  void readBuffers(std::vector<glm::vec3>& vertices, std::vector<unsigned int>& indices) {
    unsigned int vertexCount = 0;
    unsigned int indexCount  = 0;

    // Read vertex count.
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_count_buffer_);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &vertexCount);

    // Read index count.
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, index_count_buffer_);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &indexCount);

    // Resize output vectors.
    vertices.resize(vertexCount);
    indices.resize(indexCount);

    // Read vertex data.
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_buffer_);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec3) * vertexCount, vertices.data());

    // Read index data.
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, index_buffer_);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int) * indexCount, indices.data());
  }

  void write_obj(const std::string& outputPath, const std::vector<glm::vec3>& vertices,
                const std::vector<unsigned int>& indices) {
    // Create ASSIMP scene.
    aiScene scene;
    scene.mRootNode             = new aiNode();
    scene.mMeshes               = new aiMesh*[1];
    scene.mMeshes[0]            = new aiMesh();
    scene.mNumMeshes            = 1;
    scene.mRootNode->mMeshes    = new unsigned int[1]{0};
    scene.mRootNode->mNumMeshes = 1;

    aiMesh* mesh = scene.mMeshes[0];

    // Fill vertices.
    mesh->mVertices    = new aiVector3D[vertices.size()];
    mesh->mNumVertices = static_cast<unsigned int>(vertices.size());
    for (size_t i = 0; i < vertices.size(); ++i) {
      mesh->mVertices[i] = aiVector3D(vertices[i].x, vertices[i].y, vertices[i].z);
    }

    // Fill faces.
    mesh->mFaces    = new aiFace[indices.size() / 3];
    mesh->mNumFaces = indices.size() / 3;
    for (size_t i = 0; i < indices.size(); i += 3) {
      aiFace& face     = mesh->mFaces[i / 3];
      face.mIndices    = new unsigned int[3]{indices[i], indices[i + 1], indices[i + 2]};
      face.mNumIndices = 3;
    }

    // Export to .obj using ASSIMP.
    Assimp::Exporter exporter;
    if (exporter.Export(&scene, "obj", outputPath) != AI_SUCCESS) {
      throw std::runtime_error("Failed to export mesh: " + std::string(exporter.GetErrorString()));
    }

    // Cleanup ASSIMP structures.
    delete scene.mMeshes[0];
    delete[] scene.mMeshes;
    delete scene.mRootNode;
  }
};
}  // namespace resin
