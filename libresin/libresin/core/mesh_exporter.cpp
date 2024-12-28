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
  glDeleteBuffers(1, &vertex_count_buffer);
  glDeleteBuffers(1, &normal_buffer);
}

void MeshExporter::export_to_obj(const std::string& output_path, const glm::vec3& grid_origin,
                                 const glm::vec3& voxel_size, const unsigned int march_res) {
  // Set up compute shader.
  compute_shader_program_.bind();
  compute_shader_program_.set_uniform("gridOrigin", grid_origin);
  compute_shader_program_.set_uniform("voxelSize", voxel_size);

  // Dispatch compute shader.
  glDispatchCompute(march_res / 8, march_res / 8, march_res / 8);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  read_buffers();
  write_obj(output_path);
}

void MeshExporter::initialize_buffers() {
  // Create buffers
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * 1000000, nullptr, GL_STREAM_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_buffer);

  glGenBuffers(1, &vertex_count_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_count_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), nullptr, GL_STREAM_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vertex_count_buffer);

  glGenBuffers(1, &normal_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, normal_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * 1000000, nullptr, GL_STREAM_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, normal_buffer);
}

void MeshExporter::read_buffers() {
  unsigned int vertex_count = 0;

  // Read vertex count.
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_count_buffer);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &vertex_count);

  // Resize output vectors.
  vertices_.resize(vertex_count);
  normals_.resize(vertex_count);

  // Read vertex data.
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_buffer);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * vertex_count, vertices_.data());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, normal_buffer);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * vertex_count, normals_.data());

}

void MeshExporter::write_obj(const std::string& output_path) {
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
  mesh->mVertices = new aiVector3D[vertices_.size()];
  mesh->mNormals = new aiVector3D[normals_.size()];
  mesh->mNumVertices = static_cast<unsigned int>(vertices_.size());
  for (size_t i = 0; i < vertices_.size(); ++i) {
    mesh->mVertices[i] = aiVector3D(vertices_[i].x, vertices_[i].y, vertices_[i].z);
    mesh->mNormals[i] = aiVector3D(normals_[i].x, normals_[i].y, normals_[i].z);
  }

  mesh->mFaces = new aiFace[vertices_.size() / 3];
  mesh->mNumFaces = vertices_.size() / 3;
  for (unsigned int i = 0; i < vertices_.size(); i += 3) {
    aiFace& face = mesh->mFaces[i / 3];
    face.mIndices = new unsigned int[3]{ i + 1, i, i + 2 }; // counterclockwise winding order
    face.mNumIndices = 3;
  }


  // Export to .obj using ASSIMP.
  Assimp::Exporter exporter;
  if (exporter.Export(&scene, "objnomtl", output_path) != AI_SUCCESS) {
    throw std::runtime_error("Failed to export mesh: " + std::string(exporter.GetErrorString()));
  }

}
}