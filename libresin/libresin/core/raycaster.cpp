#include <glad/gl.h>

#include <libresin/core/raycaster.hpp>

namespace resin {

Raycaster::Raycaster() : vertex_array_(0), vertex_buffer_(0), index_buffer_(0) {
  // Generate VAO
  glGenVertexArrays(1, &vertex_array_);
  glBindVertexArray(vertex_array_);

  // Generate VBO and load data into it
  glGenBuffers(1, &vertex_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), kVertices.data(), GL_STATIC_DRAW);

  // Set vertex attrib pointers
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

  // Generate indices
  glGenBuffers(1, &index_buffer_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices.data(), GL_STATIC_DRAW);
}

Raycaster::~Raycaster() {
  glDeleteBuffers(1, &index_buffer_);
  glDeleteBuffers(1, &vertex_buffer_);
  glDeleteVertexArrays(1, &vertex_array_);
}

void Raycaster::bind() const { glBindVertexArray(vertex_array_); }

void Raycaster::unbind() const { glBindVertexArray(0); }  // NOLINT

void Raycaster::draw_call() const {  // NOLINT
  glDrawElements(GL_TRIANGLES, kIndices.size(), GL_UNSIGNED_INT, nullptr);
}

}  // namespace resin