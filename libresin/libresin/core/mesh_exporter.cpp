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
MeshExporter::MeshExporter(ShaderResourceManager& shader_manager, unsigned int resolution)
    : shader_manager_(shader_manager), shader_resource_(), resolution_(resolution), scene_(new aiScene()) {
  const std::filesystem::path path = std::filesystem::current_path() / "assets";
  shader_resource_                 = *shader_manager_.get_res(path / "marching_cubes.comp");
  initialize_buffers();
}

void MeshExporter::setup_scene(const glm::vec3& bb_start, const glm::vec3& bb_end, const SDFTree& sdf_tree) {
  execute_shader(bb_start, bb_end, sdf_tree);
  read_buffers();
  create_scene();
}

MeshExporter::~MeshExporter() {
  glDeleteBuffers(1, &edges_lookup_);
  glDeleteBuffers(1, &triangles_lookup_);
  glDeleteBuffers(1, &vertex_buffer_);
  glDeleteBuffers(1, &vertex_count_buffer_);
  glDeleteBuffers(1, &normal_buffer_);
  glDeleteBuffers(1, &uv_buffer_);
  delete scene_;
}


void MeshExporter::export_mesh(const std::string& output_path, const std::string& format) {
  Assimp::Exporter exporter;
  if (exporter.Export(scene_, format, output_path) != AI_SUCCESS) {
    Logger::err("Failed to export mesh asset '{}'", output_path);
  }}

void MeshExporter::execute_shader(const glm::vec3 bb_start, const glm::vec3 bb_end, SDFTree sdf_tree) {
  UniformBuffer ubo(sdf_tree.max_nodes_count());
  shader_resource_.set_ext_defi("SDF_CODE", sdf_tree.gen_shader_code());
  shader_resource_.set_ext_defi("MAX_UBO_NODE_COUNT", std::to_string(ubo.max_count()));
  ubo.bind();
  ubo.set(sdf_tree);
  ubo.unbind();

  ComputeShaderProgram compute_shader_program("marching_cubes", shader_resource_);

  // Set up compute shader
  compute_shader_program.bind();
  compute_shader_program.set_uniform("boundingBoxStart", bb_start);
  compute_shader_program.set_uniform("boundingBoxEnd", bb_end);
  compute_shader_program.set_uniform("marchRes", resolution_);

  // Dispatch compute shader.
  glDispatchCompute(resolution_ / 8, resolution_ / 8, resolution_ / 8);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);  // add GL_SHADER_IMAGE_ACCESS_BARRIER_BIT later for texture
}
void MeshExporter::initialize_buffers() {
  const size_t max_vertices = std::pow(resolution_, 3) * 3 * 5;  // max 5 triangles in each cuboid

  glGenBuffers(1, &edges_lookup_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, edges_lookup_);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 256, edge_table, GL_STREAM_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, edges_lookup_);

  glGenBuffers(1, &triangles_lookup_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangles_lookup_);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 4096, tri_table, GL_STREAM_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triangles_lookup_);

  glGenBuffers(1, &vertex_buffer_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_buffer_);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * max_vertices, nullptr, GL_STREAM_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vertex_buffer_);

  glGenBuffers(1, &vertex_count_buffer_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_count_buffer_);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), nullptr, GL_STREAM_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, vertex_count_buffer_);

  glGenBuffers(1, &normal_buffer_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, normal_buffer_);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * max_vertices, nullptr, GL_STREAM_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, normal_buffer_);

  glGenBuffers(1, &uv_buffer_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uv_buffer_);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec2) * max_vertices, nullptr, GL_STREAM_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, uv_buffer_);
}

void MeshExporter::read_buffers() {
  unsigned int vertex_count = 0;

  // Read vertex count.
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_count_buffer_);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &vertex_count);

  // Resize output vectors.
  vertices_.resize(vertex_count);
  normals_.resize(vertex_count);
  uvs_.resize(vertex_count);

  // Read vertex data.
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_buffer_);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * vertex_count, vertices_.data());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, normal_buffer_);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * vertex_count, normals_.data());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uv_buffer_);
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

}  // namespace resin