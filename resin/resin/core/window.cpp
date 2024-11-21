#include <GLFW/glfw3.h>

#include <libresin/utils/logger.hpp>
#include <memory>
#include <resin/core/graphics_context.hpp>
#include <resin/core/window.hpp>
#include <resin/event/event.hpp>
#include <resin/event/window_events.hpp>
#include <stdexcept>
#include <string_view>

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
  if (glfw_window_count_ == 0) {
    api_init();
  }

#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  // TODO(SDF-72): proper window creation for fullscreen
  window_ptr_ = glfwCreateWindow(static_cast<int>(properties_.width), static_cast<int>(properties_.height),
                                 properties_.title.c_str(), nullptr, nullptr);
  if (window_ptr_ == nullptr) {  // MAYBE: semantic exception class (i.e. window_creation_error)?
    throw std::runtime_error("Unable to create window");
  }
  ++glfw_window_count_;

  context_ = std::make_unique<GraphicsContext>(window_ptr_);
  context_->init();

  glfwSetWindowUserPointer(window_ptr_, &properties_);

  if (properties_.x && properties_.y) {
    glfwSetWindowPos(window_ptr_, *properties_.x, *properties_.y);
  } else {
    int x, y;  // NOLINT
    glfwGetWindowPos(window_ptr_, &x, &y);
    properties_.x = x;
    properties_.y = y;
  }

  glfwSwapInterval(properties_.vsync ? 1 : 0);

  if (properties_.eventDispatcher.has_value()) {
    set_glfw_callbacks();
  } else {
    Logger::warn("Window {} created without event dispatcher!", properties_.title);
  }

  Logger::info("Created window {} ({} x {})", properties_.title, properties_.width, properties_.height);
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

  glfwSetKeyCallback(window_ptr_, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
    const WindowProperties& properties = *static_cast<WindowProperties*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_Y) {
      WindowTestEvent window_test_event;
      properties.eventDispatcher->get().dispatch(window_test_event);
    }
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
  glfwSetWindowPos(window_ptr_, pos.x, pos.y);
  properties_.x = pos.x;
  properties_.y = pos.y;
}

void Window::set_dimensions(glm::uvec2 dimensions) {
  glfwSetWindowSize(window_ptr_, static_cast<int>(dimensions.x), static_cast<int>(dimensions.y));
  properties_.width  = dimensions.x;
  properties_.height = dimensions.y;
}

void Window::set_vsync(bool vsync) {
  glfwSwapInterval(vsync ? 1 : 0);
  properties_.vsync = vsync;
}

void Window::set_fullscreen(bool fullscreen) {
  properties_.fullscreen = fullscreen;

  // TODO(SDF-72): proper handling of properties, we have to remember old x,y,width,height somewhere
  if (fullscreen) {
    GLFWmonitor* monitor    = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwSetWindowMonitor(window_ptr_, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
  } else {
    glfwSetWindowMonitor(window_ptr_, nullptr, *properties_.x, *properties_.y, static_cast<int>(properties_.width),
                         static_cast<int>(properties_.height), 0);
  }
}

}  // namespace resin
