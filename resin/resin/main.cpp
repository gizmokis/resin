#include <GLFW/glfw3.h>
#include <assimp/version.h>
#include <glad/gl.h>
#include <imgui/imgui.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>
#include <libresin/utils/path.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <print>
#include <resin/resin.hpp>
#include <version/version.hpp>

int main() {
  const size_t max_logs_backups = 4;
  auto logs_dir                 = resin::get_executable_dir();
  logs_dir.append("logs");
  std::filesystem::create_directory(logs_dir);

  resin::Logger::get_instance().set_abs_build_path(RESIN_BUILD_ABS_PATH);
  resin::Logger::get_instance().add_scribe(std::make_unique<resin::TerminalLoggerScribe>());
  resin::Logger::get_instance().add_scribe(
      std::make_unique<resin::RotatedFileLoggerScribe>(logs_dir, max_logs_backups));

  resin::Logger::info("Resin version: {0}.{1}.{2}({3})", RESIN_VERSION_MAJOR, RESIN_VERSION_MINOR, RESIN_VERSION_PATCH,
                      RESIN_IS_STABLE ? "stable" : "unstable");
  resin::Logger::info("ImGui version: {0}", IMGUI_VERSION);

  resin::Logger::info("GLFW version: {0}.{1}.{2}", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);
  resin::Logger::info("GLM version: {0}.{1}.{2}", GLM_VERSION_MAJOR, GLM_VERSION_MINOR, GLM_VERSION_REVISION);
  resin::Logger::info("nlohmann/json version: {0}.{1}.{2}", NLOHMANN_JSON_VERSION_MAJOR, NLOHMANN_JSON_VERSION_MINOR,
                      NLOHMANN_JSON_VERSION_PATCH);
  resin::Logger::info("assimp version: {0}.{1}.{2}", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionPatch());

  resin::Resin::instance().run();

  return 0;
}
