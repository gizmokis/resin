#ifndef RESIN_GRAPHICS_CONTEXT_HPP
#define RESIN_GRAPHICS_CONTEXT_HPP

#include <GLFW/glfw3.h>

namespace resin {

class GraphicsContext {
 public:
  explicit GraphicsContext(GLFWwindow* window);
  ~GraphicsContext() = default;

  void init();
  void swap_buffers();

  GraphicsContext(const GraphicsContext&)            = delete;
  GraphicsContext(GraphicsContext&&)                 = delete;
  GraphicsContext& operator=(const GraphicsContext&) = delete;
  GraphicsContext& operator=(GraphicsContext&&)      = delete;

 private:
  GLFWwindow* window_ptr_;
};

}  // namespace resin

#endif  // RESIN_GRAPHICS_CONTEXT_HPP
