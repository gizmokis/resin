#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <imgui/imgui.h>

#include <glm/glm.hpp>
#include <libresin/resin.hpp>
#include <print>
#include <version/version.hpp>

int main() {
  resin::say_resin();
  std::println("Project version: {0}.{1}.{2}({3})", RESIN_VERSION_MAJOR, RESIN_VERSION_MINOR, RESIN_VERSION_PATCH,
               RESIN_IS_STABLE ? "stable" : "unstable");

  std::println("ImGui version: {0}", IMGUI_VERSION);
  std::println("GLFW version: {0}.{1}.{2}", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);
  std::println("GLM version: {0}.{1}.{2}", GLM_VERSION_MAJOR, GLM_VERSION_MINOR, GLM_VERSION_REVISION);

  constexpr int kWidth = 800;
  constexpr int kHeight = 600;
  glfwInit();
  GLFWwindow* window = glfwCreateWindow(kWidth, kHeight, "Test", nullptr, nullptr);
  glfwMakeContextCurrent(window);

  const int glad_version = gladLoadGL(glfwGetProcAddress);
  if (glad_version == 0) {
    std::println("Failed to initialize OpenGL context");
    return -1;
  }

  std::println("GLAD version: {0}.{1}", GLAD_VERSION_MAJOR(glad_version), GLAD_VERSION_MINOR(glad_version));

  glfwDestroyWindow(window);

  return 0;
}
