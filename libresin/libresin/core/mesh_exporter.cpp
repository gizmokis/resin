#include <stdexcept>
#include <string>
#include <vector>

#include <assimp/Exporter.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/vector3.h>
#include <glm/vec3.hpp>

#include <libresin/core/mesh_exporter.hpp>

namespace resin {
MeshExporter::MeshExporter(const ComputeShaderProgram& compute_shader_program):
  compute_shader_program_(compute_shader_program){
  initialize_buffers();
}

MeshExporter::~MeshExporter() {
  glDeleteBuffers(1, &vertex_buffer);
}

void MeshExporter::export_to_obj(const std::string& output_path, const glm::vec3& grid_origin,
                                 const glm::vec3& voxel_size, const unsigned int march_res) {
  // Set up compute shader.
  compute_shader_program_.bind();
  compute_shader_program_.set_uniform("gridOrigin", grid_origin);
  compute_shader_program_.set_uniform("voxelSize", voxel_size);
  //compute_shader_program_.set_uniform("gridResolution", grid_resolution);

  // Dispatch compute shader.
  glDispatchCompute(march_res / 8, march_res / 8, march_res / 8);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // Read back data.
  std::vector<glm::vec4> vertices;
  read_buffers(vertices);

  // Export mesh.
  write_obj(output_path, vertices);
}

void MeshExporter::initialize_buffers() {
  // Create buffers
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * 1000000, nullptr, GL_STREAM_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_buffer);

  // Create buffers for vertex and index counts.
  glGenBuffers(1, &vertex_count_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_count_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vertex_count_buffer);
}

void MeshExporter::read_buffers(std::vector<glm::vec4>& vertices) const {
  unsigned int vertex_count = 0;

  // Read vertex count.
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_count_buffer);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &vertex_count);

  // Resize output vectors.
  vertices.resize(vertex_count);

  // Read vertex data.
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_buffer);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * vertex_count, vertices.data());
}

void MeshExporter::write_obj(const std::string& output_path, const std::vector<glm::vec4>& vertices) {
  // Create ASSIMP scene.
  aiScene scene;
  scene.mRootNode = new aiNode();
  scene.mMeshes = new aiMesh*[1];
  scene.mMeshes[0] = new aiMesh();
  scene.mNumMeshes = 1;
  scene.mRootNode->mMeshes = new unsigned int[1]{ 0 };
  scene.mRootNode->mNumMeshes = 1;

  scene.mMaterials = new aiMaterial*[ 1 ];
  scene.mMaterials[ 0 ] = nullptr;
  scene.mNumMaterials = 1;

  scene.mMaterials[ 0 ] = new aiMaterial();



  aiMesh* mesh = scene.mMeshes[0];

  // Fill vertices.
  mesh->mVertices = new aiVector3D[vertices.size()];
  mesh->mNumVertices = static_cast<unsigned int>(vertices.size());
  for (size_t i = 0; i < vertices.size(); ++i) {
    mesh->mVertices[i] = aiVector3D(vertices[i].x, vertices[i].y, vertices[i].z);
  }

  mesh->mFaces = new aiFace[vertices.size() / 3];
  mesh->mNumFaces = vertices.size() / 3;
  for (unsigned int i = 0; i < vertices.size(); i += 3) {
    aiFace& face = mesh->mFaces[i / 3];
    face.mIndices = new unsigned int[3]{ i, i + 1, i + 2 };
    face.mNumIndices = 3;
  }


  // Export to .obj using ASSIMP.
  Assimp::Exporter exporter;
  if (exporter.Export(&scene, "objnomtl", output_path) != AI_SUCCESS) {
    throw std::runtime_error("Failed to export mesh: " + std::string(exporter.GetErrorString()));
  }

  // Cleanup ASSIMP structures.
  // delete scene.mMeshes[0];
  // delete[] scene.mMeshes;
  // delete scene.mRootNode;
}
}