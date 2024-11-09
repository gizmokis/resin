#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <imgui/imgui.h>

#include <glm/glm.hpp>
#include <libresin/resin.hpp>
#include <libresin/utils/logger.hpp>
#include <memory>
#include <print>
#include <version/version.hpp>

int main() {
  resin::Logger::get_instance().add_scribe(std::make_unique<resin::TerminalLoggerScribe>());

  resin::Logger::info("Project version: {0}.{1}.{2}({3})", RESIN_VERSION_MAJOR, RESIN_VERSION_MINOR,
                      RESIN_VERSION_PATCH, RESIN_IS_STABLE ? "stable" : "unstable");
  resin::Logger::info("ImGui version: {0}", IMGUI_VERSION);

  resin::Logger::info("GLFW version: {0}.{1}.{2}", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);
  resin::Logger::info("GLM version: {0}.{1}.{2}", GLM_VERSION_MAJOR, GLM_VERSION_MINOR, GLM_VERSION_REVISION);

  resin::Logger::warn("Potato");
  resin::Logger::err("Paprica");
  resin::Logger::debug("Blueberry");

  constexpr int kWidth  = 800;
  constexpr int kHeight = 600;

  glfwInit();
  GLFWwindow* window = glfwCreateWindow(kWidth, kHeight, "Test", nullptr, nullptr);
  glfwMakeContextCurrent(window);

  const int glad_version = gladLoadGL(glfwGetProcAddress);
  if (glad_version == 0) {
    resin::Logger::err("Failed to initialize OpenGL context");
    return -1;
  }

  resin::Logger::info("GLAD version: {0}.{1}", GLAD_VERSION_MAJOR(glad_version), GLAD_VERSION_MINOR(glad_version));

  glfwDestroyWindow(window);

  return 0;
}
