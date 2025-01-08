#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/vector3.h>

#include <assimp/Exporter.hpp>
#include <glm/vec3.hpp>
#include <libresin/core/mesh_exporter.hpp>
#include <libresin/core/shader.hpp>
#include <libresin/core/shader_storage_buffer.hpp>
#include <libresin/core/uniform_buffer.hpp>
#include <memory>
#include <string>
#include <vector>

namespace resin {

MeshExporter::MeshExporter(unsigned int resolution)
    : resolution_(resolution),
      shader_resource_(*shader_manager_->get_res(std::filesystem::current_path() / "assets/marching_cubes.comp")),
      scene_(new aiScene()) {
  initialize_buffers();
}

void MeshExporter::setup_scene(const glm::vec3& bb_start, const glm::vec3& bb_end, SDFTree& sdf_tree) {
  execute_shader(bb_start, bb_end, sdf_tree);
  read_buffers();
  create_scene();
}

MeshExporter::~MeshExporter() { delete scene_; }

void MeshExporter::export_mesh(const std::string& output_path, const std::string& format) {
  Assimp::Exporter exporter;
  if (exporter.Export(scene_, format, output_path) != AI_SUCCESS) {
    Logger::err("Failed to export mesh asset '{}'", output_path);
  } else {
    Logger::info("Successfully exported mesh asset '{}'", output_path);
  }
}

void MeshExporter::execute_shader(const glm::vec3 bb_start, const glm::vec3 bb_end, SDFTree& sdf_tree) {
  UniformBuffer ubo(sdf_tree.max_nodes_count(), 1);
  shader_resource_.set_ext_defi("SDF_CODE", sdf_tree.gen_shader_code());
  shader_resource_.set_ext_defi("MAX_UBO_NODE_COUNT", std::to_string(ubo.max_count()));
  ComputeShaderProgram compute_shader_program("marching_cubes", shader_resource_);

  // Set up compute shader
  ubo.bind();
  ubo.set(sdf_tree);
  ubo.unbind();
  compute_shader_program.bind();

  compute_shader_program.set_uniform("u_boundingBoxStart", bb_start);
  compute_shader_program.set_uniform("u_boundingBoxEnd", bb_end);
  compute_shader_program.set_uniform("u_marchRes", resolution_);
  compute_shader_program.set_uniform("u_farPlane", 100.f);  // TODO remove this uniform

  // Dispatch compute shader.
  glDispatchCompute(resolution_ / 8, resolution_ / 8, resolution_ / 8);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);  // add GL_SHADER_IMAGE_ACCESS_BARRIER_BIT later for texture
  compute_shader_program.unbind();
}

void MeshExporter::initialize_buffers() {
  const GLsizei max_vertices =
      static_cast<GLsizei>(std::pow(resolution_, 3) * 3 * 5);  // max 5 triangles per each cuboid

  edges_lookup_buffer_ = std::make_unique<ShaderStorageBuffer>(sizeof(edge_table_), 0);
  edges_lookup_buffer_->set_data(edge_table_, sizeof(edge_table_));

  triangles_lookup_buffer_ = std::make_unique<ShaderStorageBuffer>(sizeof(tri_table_), 1);
  triangles_lookup_buffer_->set_data(tri_table_, sizeof(tri_table_));

  vertex_buffer_       = std::make_unique<ShaderStorageBuffer>(sizeof(glm::vec4) * max_vertices, 2, GL_STREAM_READ);
  vertex_count_buffer_ = std::make_unique<ShaderStorageBuffer>(sizeof(GLuint), 3, GL_STREAM_READ);
  GLuint init          = 0;
  vertex_count_buffer_->set_data(&init, sizeof(GLuint));

  normal_buffer_ = std::make_unique<ShaderStorageBuffer>(sizeof(glm::vec4) * max_vertices, 4, GL_STREAM_READ);
  uv_buffer_     = std::make_unique<ShaderStorageBuffer>(sizeof(glm::vec2) * max_vertices, 5, GL_STREAM_READ);
}

void MeshExporter::read_buffers() {
  GLuint vertex_count = 0;

  vertex_count_buffer_->get_data(&vertex_count, sizeof(GLuint));

  vertices_.resize(vertex_count);
  normals_.resize(vertex_count);
  uvs_.resize(vertex_count);

  vertex_buffer_->get_data(vertices_.data(), static_cast<GLsizeiptr>(sizeof(glm::vec4) * vertex_count));
  normal_buffer_->get_data(normals_.data(), static_cast<GLsizeiptr>(sizeof(glm::vec4) * vertex_count));
  uv_buffer_->get_data(uvs_.data(), static_cast<GLsizeiptr>(sizeof(glm::vec2) * vertex_count));
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