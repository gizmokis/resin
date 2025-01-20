#ifndef RESIN_HPP
#define RESIN_HPP

#include <chrono>
#include <cstdint>
#include <glm/ext/vector_float2.hpp>
#include <libresin/core/camera.hpp>
#include <libresin/core/framebuffer.hpp>
#include <libresin/core/material.hpp>
#include <libresin/core/raycaster.hpp>
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/shader.hpp>
#include <libresin/core/uniform_buffer.hpp>
#include <memory>
#include <resin/camera/first_person_camera_operator.hpp>
#include <resin/camera/orbiting_camera_operator.hpp>
#include <resin/core/key_codes.hpp>
#include <resin/core/window.hpp>
#include <resin/event/event.hpp>
#include <resin/event/key_events.hpp>
#include <resin/event/mouse_events.hpp>
#include <resin/event/window_events.hpp>
#include <resin/imgui/gizmo.hpp>
#include <resin/imgui/material.hpp>
#include <resin/imgui/types.hpp>
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
  void setup_material_framebuffers();

  void run();
  void init_gl();
  void update(duration_t delta);
  void gui(duration_t delta);
  void render_viewport();
  void render_material_image(ImageFramebuffer& fb);
  void render_material_images();

  // events
  bool on_window_close(WindowCloseEvent& e);
  bool on_window_resize(WindowResizeEvent& e);
  bool on_mouse_btn_pressed(MouseButtonPressedEvent& e);
  bool on_mouse_btn_released(MouseButtonReleasedEvent& e);
  bool on_scroll(MouseScrollEvent& e);
  bool on_key_pressed(KeyPressedEvent& e);
  bool on_key_released(KeyReleasedEvent& e);

  // viewport actions -- these methods mutate the viewport state
  bool update_viewport_active(bool is_viewport_focused);
  bool draw_transform_gizmo();
  bool draw_camera_gizmo(float dt);
  bool update_camera_distance();
  bool switch_ortho();
  bool activate_first_person_camera(glm::vec2 mouse_pos);
  bool deactivate_first_person_camera();
  bool start_moving_first_person_camera(key::Code key_code);
  bool stop_moving_first_person_camera(key::Code key_code);
  bool activate_orbiting_camera(glm::vec2 mouse_pos);
  bool deactivate_orbiting_camera();
  bool update_camera_operators(float dt);
  bool zoom_camera(glm::vec2 offset);
  bool select_node(glm::vec2 relative_pos);
  bool start_interpolation();
  bool interpolate(float dt);

 public:
  static constexpr duration_t kTickTime = 16666us;  // 60 TPS = 16.6(6) ms/t

 private:
  static constexpr size_t kMaterialNodeImageSize = 32;
  static constexpr size_t kMaterialImageSize     = 64;
  static constexpr size_t kMaterialMainImageSize = 160;

  EventDispatcher dispatcher_;
  ShaderResourceManager& shader_resource_manager_ = ResourceManagers::shader_manager();

  SDFTree sdf_tree_;

  enum class ViewportState : uint8_t {
    InactiveIdle,
    ActiveIdle,
    FirstPersonCamera,
    OrbitingCamera,
    GizmoCamera,
    GizmoTransform,
    CameraInterpolation,
    _Count  // NOLINT
  };
  ViewportState current_viewport_state_;

  std::optional<IdView<SDFTreeNodeId>> selected_node_;
  std::optional<IdView<MaterialId>> selected_material_;

  std::unique_ptr<Window> window_;
  std::unique_ptr<Raycaster> raycaster_;
  std::unique_ptr<RenderingShaderProgram> shader_;
  std::unique_ptr<RenderingShaderProgram> grid_shader_;
  std::unique_ptr<RenderingShaderProgram> material_img_shader_;
  std::unique_ptr<PrimitiveUniformBuffer> primitive_ubo_;
  std::unique_ptr<MaterialUniformBuffer> material_ubo_;
  std::unique_ptr<ViewportFramebuffer> framebuffer_;
  std::unique_ptr<ImGui::resin::LazyMaterialImageFramebuffers> material_images_;

  glm::vec2 viewport_pos_;

  std::unique_ptr<Camera> camera_;
  std::unique_ptr<PointLight> point_light_;
  std::unique_ptr<DirectionalLight> directional_light_;
  // TEMP(SDF-131): remove
  void material_inspect(Material& mat, std::string_view name);
  std::unique_ptr<Material> sphere_mat_, cube_mat_, torus_mat_, capsule_mat_, link_mat_, ellipsoid_mat_, pyramid_mat_,
      cylinder_mat_, prism_mat_;

  bool use_local_gizmos_{false};
  bool is_grid_{true};
  float grid_spacing_ = 1.0;

  float camera_distance_;

  ImGui::resin::gizmo::Operation gizmo_operation_;

  OrbitingCameraOperator orbiting_camera_operator_;
  FirstPersonCameraOperator first_person_camera_operator_;

  bool running_   = true;
  bool minimized_ = false;

  duration_t time_ = 0ns;
  uint16_t fps_ = 0, tps_ = 0;
  friend int ::main();
};

}  // namespace resin

#endif  // RESIN_HPP
