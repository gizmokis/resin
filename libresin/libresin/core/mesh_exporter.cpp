#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/vector3.h>

#include <assimp/Exporter.hpp>
#include <glm/vec3.hpp>
#include <libresin/core/mesh_exporter.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace resin {
MeshExporter::MeshExporter(const ComputeShaderProgram& compute_shader_program)
    : compute_shader_program_(compute_shader_program), scene_(new aiScene()) {}

MeshExporter::~MeshExporter() {
  glDeleteBuffers(1, &edges_lookup);
  glDeleteBuffers(1, &triangles_lookup);
  glDeleteBuffers(1, &vertex_buffer);
  glDeleteBuffers(1, &vertex_count_buffer);
  glDeleteBuffers(1, &normal_buffer);
  glDeleteBuffers(1, &uv_buffer);
  delete scene_;
}

void MeshExporter::export_mesh(const std::string& output_path, const std::string& format, const glm::vec3& bb_start,
                               const glm::vec3& bb_end, const unsigned int march_res) {
  initialize_buffers(march_res);
  execute_shader(bb_start, bb_end, march_res);
  read_buffers();
  create_scene();
  export_scene(output_path, format);
}

void MeshExporter::execute_shader(const glm::vec3 bb_start, const glm::vec3 bb_end,
                                  const unsigned int march_res) const {
  // Set up compute shader
  compute_shader_program_.bind();
  compute_shader_program_.set_uniform("boundingBoxStart", bb_start);
  compute_shader_program_.set_uniform("boundingBoxEnd", bb_end);
  compute_shader_program_.set_uniform("marchRes", march_res);

  // Dispatch compute shader.
  glDispatchCompute(march_res / 8, march_res / 8, march_res / 8);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);  // add GL_SHADER_IMAGE_ACCESS_BARRIER_BIT later for texture
}
void MeshExporter::initialize_buffers(const unsigned int march_res) {
  const size_t max_vertices = std::pow(march_res, 3) * 3 * 5;  // max 5 triangles in each cuboid

  glGenBuffers(1, &edges_lookup);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, edges_lookup);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 256, edge_table, GL_STREAM_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, edges_lookup);

  glGenBuffers(1, &triangles_lookup);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangles_lookup);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 4096, tri_table, GL_STREAM_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triangles_lookup);

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * max_vertices, nullptr, GL_STREAM_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vertex_buffer);

  glGenBuffers(1, &vertex_count_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_count_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), nullptr, GL_STREAM_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, vertex_count_buffer);

  glGenBuffers(1, &normal_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, normal_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * max_vertices, nullptr, GL_STREAM_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, normal_buffer);

  glGenBuffers(1, &uv_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uv_buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec2) * max_vertices, nullptr, GL_STREAM_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, uv_buffer);
}

void MeshExporter::read_buffers() {
  unsigned int vertex_count = 0;

  // Read vertex count.
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_count_buffer);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &vertex_count);

  // Resize output vectors.
  vertices_.resize(vertex_count);
  normals_.resize(vertex_count);
  uvs_.resize(vertex_count);

  // Read vertex data.
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_buffer);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * vertex_count, vertices_.data());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, normal_buffer);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * vertex_count, normals_.data());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uv_buffer);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec2) * vertex_count, uvs_.data());
}

void MeshExporter::create_scene() {
  scene_->mRootNode             = new aiNode();
  scene_->mMeshes               = new aiMesh*[1];
  scene_->mMeshes[0]            = new aiMesh();
  scene_->mNumMeshes            = 1;
  scene_->mRootNode->mMeshes    = new unsigned int[1]{0};
  scene_->mRootNode->mNumMeshes = 1;

  scene_->mMaterials    = new aiMaterial*[1];
  scene_->mMaterials[0] = nullptr;
  scene_->mNumMaterials = 1;
  scene_->mMaterials[0] = new aiMaterial();

  aiMesh* mesh          = scene_->mMeshes[0];
  mesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;

  // Fill vertices.
  mesh->mVertices           = new aiVector3D[vertices_.size()];
  mesh->mNumVertices        = static_cast<unsigned int>(vertices_.size());
  mesh->mNormals            = new aiVector3D[normals_.size()];
  mesh->mTextureCoords[0]   = new aiVector3D[uvs_.size()];
  mesh->mNumUVComponents[0] = 2;
  for (size_t i = 0; i < vertices_.size(); ++i) {
    mesh->mVertices[i]         = aiVector3D(vertices_[i].x, vertices_[i].y, vertices_[i].z);
    mesh->mNormals[i]          = aiVector3D(normals_[i].x, normals_[i].y, normals_[i].z);
    mesh->mTextureCoords[0][i] = aiVector3D(uvs_[i].x, uvs_[i].y, 0.0f);
  }

  mesh->mFaces    = new aiFace[vertices_.size() / 3];
  mesh->mNumFaces = vertices_.size() / 3;
  for (unsigned int i = 0; i < vertices_.size(); i += 3) {
    aiFace& face     = mesh->mFaces[i / 3];
    face.mIndices    = new unsigned int[3]{i + 1, i, i + 2};  // counterclockwise winding order
    face.mNumIndices = 3;
  }
}

void MeshExporter::export_scene(const std::string& output_path, const std::string& format) const {
  Assimp::Exporter exporter;
  if (exporter.Export(scene_, format, output_path) != AI_SUCCESS) {
    throw std::runtime_error("Failed to export mesh: " + std::string(exporter.GetErrorString()));
  }
}

}  // namespace resin