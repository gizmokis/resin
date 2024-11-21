#ifndef RESIN_WINDOW_HPP
#define RESIN_WINDOW_HPP

#include <GLFW/glfw3.h>

#include <cstdint>
#include <functional>
#include <glm/vec2.hpp>
#include <memory>
#include <optional>
#include <resin/core/graphics_context.hpp>
#include <resin/event/event.hpp>
#include <string>

namespace resin {

struct WindowProperties {
  std::string title = "Resin";

  std::optional<int> x;
  std::optional<int> y;
  unsigned int width  = 1280U;  // NOLINT
  unsigned int height = 720U;   // NOLINT

  bool vsync      = false;
  bool fullscreen = false;  // TODO(SDF-72): proper fullscreen handling

  std::optional<std::reference_wrapper<EventDispatcher>> eventDispatcher;
};

class Window {
 public:
  explicit Window(WindowProperties properties);
  ~Window();

  void on_update();

  inline std::string_view title() const { return properties_.title; }
  inline glm::ivec2 pos() const { return glm::ivec2(*properties_.x, *properties_.y); }
  inline glm::uvec2 dimensions() const { return glm::uvec2(properties_.width, properties_.height); }
  inline bool vsync() const { return properties_.vsync; }
  inline bool fullscreen() const { return properties_.fullscreen; }

  void set_title(std::string_view title);
  void set_pos(glm::ivec2 pos);
  void set_dimensions(glm::uvec2 dimensions);
  void set_vsync(bool vsync);
  void set_fullscreen(bool fullscreen);

  GLFWwindow* native_window() const { return window_ptr_; }

  Window(const Window&)            = delete;
  Window(Window&&)                 = delete;
  Window& operator=(const Window&) = delete;
  Window& operator=(Window&&)      = delete;

 private:
  static void api_init();
  static void api_terminate();

  void set_glfw_callbacks() const;

 private:
  static uint8_t glfw_window_count_;
  WindowProperties properties_;
  GLFWwindow* window_ptr_;

  std::unique_ptr<GraphicsContext> context_;
};

}  // namespace resin

#endif  // RESIN_WINDOW_HPP
