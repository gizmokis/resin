#include <GLFW/glfw3.h>

#include <libresin/utils/logger.hpp>
#include <memory>
#include <print>
#include <resin/core/graphics_context.hpp>
#include <resin/core/window.hpp>
#include <resin/event/window_events.hpp>
#include <stdexcept>
#include <string_view>

#include "resin/event/event.hpp"

namespace resin {

uint8_t Window::glfw_window_count_ = 0;

static void error_callback(int error_code, const char* description) {
  Logger::err("GLFW Error #{0}: {1}", error_code, description);
}

void Window::api_init() {
  const int status = glfwInit();
  if (!status) {
    throw std::runtime_error("GLFW init failed!");
  }

  glfwSetErrorCallback(error_callback);
  Logger::debug("Api init");
}

void Window::api_terminate() {
  glfwTerminate();
  Logger::debug("Api shutdown");
}

Window::Window(WindowProperties properties) : properties_(std::move(properties)) {
  std::println("Creating window {} ({} x {})", properties_.title, properties_.width, properties_.height);

  if (glfw_window_count_ == 0) {
    api_init();
  }
#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
  glfwWindowHint(GLFW_SAMPLES, 4);

  window_ptr_ = glfwCreateWindow(static_cast<int>(properties_.width), static_cast<int>(properties_.height),
                                 properties_.title.c_str(), nullptr, nullptr);
  ++glfw_window_count_;

  context_ = std::make_unique<GraphicsContext>(window_ptr_);
  context_->init();

  glfwSetWindowUserPointer(window_ptr_, &properties_);
  glfwGetWindowPos(window_ptr_, &(properties_.x), &(properties_.y));

  if (properties_.eventDispatcher.has_value()) {
    set_glfw_callbacks();
  } else {
    Logger::warn("Window {} created without event dispatcher!", properties_.title);
  }
}

void Window::set_glfw_callbacks() const {
  glfwSetWindowCloseCallback(window_ptr_, [](GLFWwindow* window) {
    const WindowProperties& properties = *static_cast<WindowProperties*>(glfwGetWindowUserPointer(window));

    WindowCloseEvent window_close_event;
    properties.eventDispatcher->get().dispatch(window_close_event);
  });

  glfwSetWindowSizeCallback(window_ptr_, [](GLFWwindow* window, int width, int height) {
    WindowProperties& properties = *static_cast<WindowProperties*>(glfwGetWindowUserPointer(window));
    properties.width             = static_cast<unsigned int>(width);
    properties.height            = static_cast<unsigned int>(height);

    WindowResizeEvent window_resize_event(properties.width, properties.height);
    properties.eventDispatcher->get().dispatch(window_resize_event);
  });
}

Window::~Window() {
  glfwDestroyWindow(window_ptr_);
  --glfw_window_count_;

  if (glfw_window_count_ == 0) {
    api_terminate();
  }
}

void Window::on_update() {
  glfwPollEvents();
  context_->swap_buffers();
}

void Window::set_title(std::string_view title) {
  glfwSetWindowTitle(window_ptr_, title.data());
  properties_.title = title;
}

void Window::set_pos(glm::ivec2 pos) {
  // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
  glfwSetWindowPos(window_ptr_, pos.x, pos.y);
  properties_.x = pos.x;
  properties_.y = pos.y;
  // NOLINTEND(cppcoreguidelines-pro-type-union-access)
}

void Window::set_dimensions(glm::uvec2 dimensions) {
  // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
  glfwSetWindowSize(window_ptr_, static_cast<int>(dimensions.x), static_cast<int>(dimensions.y));
  properties_.width  = dimensions.x;
  properties_.height = dimensions.y;
  // NOLINTEND(cppcoreguidelines-pro-type-union-access)
}

void Window::set_vsync(bool vsync) {
  glfwSwapInterval(vsync ? 1 : 0);
  properties_.vsync = vsync;
}

void Window::set_fullscreen(bool fullscreen) {
  properties_.fullscreen = fullscreen;

  // TODO(kuzu): proper handling of properties, we have to remember old x,y,width,height somewhere
  if (fullscreen) {
    GLFWmonitor* monitor    = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwSetWindowMonitor(window_ptr_, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
  } else {
    glfwSetWindowMonitor(window_ptr_, nullptr, properties_.x, properties_.y, static_cast<int>(properties_.width),
                         static_cast<int>(properties_.height), 0);
  }
}

}  // namespace resin