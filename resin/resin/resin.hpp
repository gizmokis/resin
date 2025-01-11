#ifndef RESIN_HPP
#define RESIN_HPP

#include <chrono>
#include <cstdint>
#include <glm/ext/vector_float2.hpp>
#include <libresin/core/camera.hpp>
#include <libresin/core/framebuffer.hpp>
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/shader.hpp>
#include <libresin/core/uniform_buffer.hpp>
#include <memory>
#include <resin/core/window.hpp>
#include <resin/event/event.hpp>
#include <resin/event/mouse_events.hpp>
#include <resin/event/window_events.hpp>
#include <resin/imgui/gizmo.hpp>
#include <resin/imgui/transform_gizmo.hpp>
#include <resin/resources/resource_managers.hpp>

int main();

namespace resin {

using namespace std::chrono_literals;
using duration_t = std::chrono::nanoseconds;

class Resin {
 public:
  Window& main_window() const { return *window_; }
  static Resin& instance() {
    static Resin instance;
    return instance;
  }

  Resin(const Resin&)            = delete;
  Resin(Resin&&)                 = delete;
  Resin& operator=(const Resin&) = delete;
  Resin& operator=(Resin&&)      = delete;

 private:
  Resin();
  ~Resin() = default;

  void setup_shader_uniforms();

  void run();
  void update(duration_t delta);
  void gui(duration_t delta);
  void render();

  bool on_window_close(WindowCloseEvent& e);
  bool on_window_resize(WindowResizeEvent& e);
  bool on_test(WindowTestEvent& e);
  bool on_click(MouseButtonPressedEvent& e);
  bool on_left_click(glm::vec2 relative_pos);

 public:
  static constexpr duration_t kTickTime = 16666us;  // 60 TPS = 16.6(6) ms/t

 private:
  unsigned int vertex_array_, vertex_buffer_, index_buffer_;
  EventDispatcher dispatcher_;
  ShaderResourceManager& shader_resource_manager_ = ResourceManagers::shader_manager();

  SDFTree sdf_tree_;

  std::optional<IdView<SDFTreeNodeId>> selected_node_;

  std::unique_ptr<Window> window_;
  std::unique_ptr<RenderingShaderProgram> shader_;
  std::unique_ptr<PrimitiveUniformBuffer> primitive_ubo_;
  std::unique_ptr<Framebuffer> framebuffer_;

  glm::vec2 viewport_pos_;

  std::unique_ptr<Camera> camera_;
  std::unique_ptr<PointLight> point_light_;
  std::unique_ptr<DirectionalLight> directional_light_;
  std::unique_ptr<Material> cube_mat_, sphere_mat_;
  Transform camera_rig_;
  bool is_viewport_focused_{false};
  bool use_local_gizmos_{false};

  ImGui::resin::GizmoOperation gizmo_operation_;

  bool running_   = true;
  bool minimized_ = false;

  duration_t time_ = 0ns;
  uint16_t fps_ = 0, tps_ = 0;
  friend int ::main();
};

}  // namespace resin

#endif  // RESIN_HPP
