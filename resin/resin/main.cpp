#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <imgui/imgui.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <libresin/resin.hpp>
#include <libresin/utils/logger.hpp>
#include <memory>
#include <print>
#include <resin/core/window.hpp>
#include <resin/event/event.hpp>
#include <resin/event/window_events.hpp>
#include <version/version.hpp>

bool running = true;  // NOLINT

bool handle_close(resin::WindowCloseEvent& close) {
  resin::Logger::info("Handling close: {}!", close);

  running = false;

  return true;
}

bool handle_resize(resin::WindowResizeEvent& resize) {
  resin::Logger::info("Handling resize: {}!", resize);
  return false;
}

int main() {
  const size_t max_logs_backups = 4;
  auto logs_dir                 = std::filesystem::current_path();
  logs_dir.append("logs");
  std::filesystem::create_directory(logs_dir);

  resin::Logger::get_instance().set_abs_build_path(RESIN_BUILD_ABS_PATH);
  resin::Logger::get_instance().add_scribe(std::make_unique<resin::TerminalLoggerScribe>());
  resin::Logger::get_instance().add_scribe(
      std::make_unique<resin::RotatedFileLoggerScribe>(logs_dir, max_logs_backups));

  resin::Logger::info("Project version: {0}.{1}.{2}({3})", RESIN_VERSION_MAJOR, RESIN_VERSION_MINOR,
                      RESIN_VERSION_PATCH, RESIN_IS_STABLE ? "stable" : "unstable");
  resin::Logger::info("ImGui version: {0}", IMGUI_VERSION);

  resin::Logger::info("GLFW version: {0}.{1}.{2}", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);
  resin::Logger::info("GLM version: {0}.{1}.{2}", GLM_VERSION_MAJOR, GLM_VERSION_MINOR, GLM_VERSION_REVISION);

  resin::Logger::warn("Potato");
  resin::Logger::err("Paprica");
  resin::Logger::debug("Blueberry");

  resin::EventDispatcher dispatcher;
  dispatcher.subscribe<resin::WindowCloseEvent>(handle_close);
  dispatcher.subscribe<resin::WindowResizeEvent>(handle_resize);

  constexpr int kWidth  = 800;
  constexpr int kHeight = 600;

  resin::WindowProperties props("Resin", kWidth, kHeight);
  props.eventDispatcher = dispatcher;
  {
    resin::Window window(props);

    while (running) {
      window.on_update();
    }
  }

  return 0;
}
