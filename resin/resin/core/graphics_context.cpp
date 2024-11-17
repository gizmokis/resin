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
}

void GraphicsContext::swap_buffers() { glfwSwapBuffers(window_ptr_); }

}  // namespace resin
