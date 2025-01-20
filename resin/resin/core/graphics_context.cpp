#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <libresin/utils/logger.hpp>
#include <print>
#include <resin/core/graphics_context.hpp>
#include <stdexcept>

namespace resin {

GraphicsContext::GraphicsContext(GLFWwindow* window) : window_ptr_(window) {
  if (!window) {
    throw std::invalid_argument("Cannot create context from null window");
  }
}

void GraphicsContext::init() {
  glfwMakeContextCurrent(window_ptr_);
  const int glad_version = gladLoadGL(glfwGetProcAddress);
  if (glad_version == 0) {
    throw std::runtime_error("Failed to initialize OpenGL context");
  }

  resin::Logger::debug("GLAD version: {0}.{1}", GLAD_VERSION_MAJOR(glad_version), GLAD_VERSION_MINOR(glad_version));
  resin::Logger::info("OpenGL info:");
  resin::Logger::info("\tVendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
  resin::Logger::info("\tRenderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
  resin::Logger::info("\tVersion: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
  GLint max_uniform_block_size     = 0;
  GLint max_uniform_block_bindings = 0;
  glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &max_uniform_block_size);
  glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &max_uniform_block_bindings);
  resin::Logger::info("\tMax uniform block size: {}", max_uniform_block_size);
  resin::Logger::info("\tMax uniform block bindings: {}", max_uniform_block_bindings);
}

void GraphicsContext::swap_buffers() { glfwSwapBuffers(window_ptr_); }

}  // namespace resin
