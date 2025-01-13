#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <glm/ext.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>
#include <libresin/core/camera.hpp>
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/transform.hpp>
#include <libresin/core/uniform_buffer.hpp>
#include <libresin/utils/enum_mapper.hpp>
#include <libresin/utils/logger.hpp>
#include <nfd/nfd.hpp>
#include <optional>
#include <resin/camera/first_person_camera_operator.hpp>
#include <resin/core/key_codes.hpp>
#include <resin/core/mouse_codes.hpp>
#include <resin/core/window.hpp>
#include <resin/dialog/file_dialog.hpp>
#include <resin/event/event.hpp>
#include <resin/event/key_events.hpp>
#include <resin/event/mouse_events.hpp>
#include <resin/event/window_events.hpp>
#include <resin/imgui/gizmo.hpp>
#include <resin/imgui/node_edit.hpp>
#include <resin/imgui/sdf_tree.hpp>
#include <resin/imgui/transform_edit.hpp>
#include <resin/imgui/viewport.hpp>
#include <resin/resin.hpp>
#include <string_view>

namespace resin {

Resin::Resin()
    : vertex_array_(0),
      vertex_buffer_(0),
      index_buffer_(0),
      viewport_pos_(),
      gizmo_operation_(ImGui::resin::gizmo::Operation::Translation) {
  dispatcher_.subscribe<WindowCloseEvent>(BIND_EVENT_METHOD(on_window_close));
  dispatcher_.subscribe<WindowResizeEvent>(BIND_EVENT_METHOD(on_window_resize));
  dispatcher_.subscribe<MouseButtonPressedEvent>(BIND_EVENT_METHOD(on_mouse_btn_pressed));
  dispatcher_.subscribe<MouseButtonReleasedEvent>(BIND_EVENT_METHOD(on_mouse_btn_released));
  dispatcher_.subscribe<KeyPressedEvent>(BIND_EVENT_METHOD(on_key_pressed));
  dispatcher_.subscribe<KeyReleasedEvent>(BIND_EVENT_METHOD(on_key_released));
  dispatcher_.subscribe<ScrollEvent>(BIND_EVENT_METHOD(on_scroll));

  {
    WindowProperties properties;
    properties.eventDispatcher = dispatcher_;

    window_ = std::make_unique<Window>(std::move(properties));
  }

  const std::filesystem::path path = std::filesystem::current_path() / "assets";

  cube_mat_   = std::make_unique<Material>(glm::vec3(0.96F, 0.25F, 0.25F));
  sphere_mat_ = std::make_unique<Material>(glm::vec3(0.25F, 0.25F, 0.96F));

  camera_             = std::make_unique<Camera>(false, 70.F, 16.F / 9.F, 0.75F, 100.F);
  glm::vec3 pos       = glm::vec3(0, 2, 3);
  glm::vec3 direction = glm::normalize(-pos);
  camera_->transform.set_local_pos(glm::vec3(0.0F, 0.0F, 5.0F));

  point_light_       = std::make_unique<PointLight>(glm::vec3(0.57F, 0.38F, 0.04F), glm::vec3(0.0F, 1.0F, 0.5F),
                                                    PointLight::Attenuation(1.0F, 0.7F, 1.8F));
  directional_light_ = std::make_unique<DirectionalLight>(glm::vec3(0.5F, 0.5F, 0.5F), 1.0F);
  directional_light_->transform.set_local_rot(glm::quatLookAt(direction, glm::vec3(0, 1, 0)));

  // TODO(anyone): temporary, move out somewhere else
  float vertices[4 * 3]   = {-1.F, -1.F, 0.F, 1.F, -1.F, 0.F, -1.F, 1.F, 0.F, 1.F, 1.F, 0.F};
  unsigned int indices[6] = {0, 1, 2, 1, 3, 2};

  // Generate VAO
  glGenVertexArrays(1, &vertex_array_);
  glBindVertexArray(vertex_array_);

  // Generate VBO and load data into it
  glGenBuffers(1, &vertex_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

  // Set vertex attrib pointers
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

  // Generate indices
  glGenBuffers(1, &index_buffer_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);

  // Example tree
  sdf_tree_.root().push_back_child<SphereNode>(SDFBinaryOperation::SmoothUnion);
  auto& group = sdf_tree_.root().push_back_child<GroupNode>(SDFBinaryOperation::SmoothUnion);
  group.push_back_child<CubeNode>(SDFBinaryOperation::SmoothUnion).transform().set_local_pos(glm::vec3(1, 1, 0));
  group.push_back_child<CubeNode>(SDFBinaryOperation::SmoothUnion).transform().set_local_pos(glm::vec3(-1, -1, 0));

  // SDF Shader
  ShaderResource frag_shader = *shader_resource_manager_.get_res(path / "test.frag");

  frag_shader.set_ext_defi("SDF_CODE", sdf_tree_.gen_shader_code());

  primitive_ubo_ = std::make_unique<PrimitiveUniformBuffer>(sdf_tree_.max_nodes_count());
  frag_shader.set_ext_defi("MAX_UBO_NODE_COUNT", std::to_string(sdf_tree_.max_nodes_count()));

  primitive_ubo_->bind();
  primitive_ubo_->set(sdf_tree_);
  primitive_ubo_->unbind();

  shader_ = std::make_unique<RenderingShaderProgram>("default", *shader_resource_manager_.get_res(path / "test.vert"),
                                                     std::move(frag_shader));
  shader_->bind_uniform_buffer("Data", *primitive_ubo_);

  framebuffer_ = std::make_unique<Framebuffer>(window_->dimensions().x, window_->dimensions().y);

  setup_shader_uniforms();
}

void Resin::setup_shader_uniforms() {
  shader_->set_uniform("u_iV", camera_->inverse_view_matrix());
  shader_->set_uniform("u_resolution", glm::vec2(framebuffer_->width(), framebuffer_->height()));
  shader_->set_uniform("u_nearPlane", camera_->near_plane());
  shader_->set_uniform("u_farPlane", camera_->far_plane());
  shader_->set_uniform("u_ortho", camera_->is_orthographic());
  shader_->set_uniform("u_camSize", camera_->height());
}

void Resin::run() {
  using clock = std::chrono::high_resolution_clock;

  duration_t lag(0ns);
  duration_t second(0ns);
  auto previous_time = clock::now();

  uint16_t frames = 0U;
  uint16_t ticks  = 0U;

  while (running_) {
    auto current_time = clock::now();
    auto delta        = current_time - previous_time;
    previous_time     = current_time;

    lag += std::chrono::duration_cast<duration_t>(delta);
    second += std::chrono::duration_cast<duration_t>(delta);

    // TODO(SDF-73): handle events when event bus present

    while (lag >= kTickTime) {
      update(kTickTime);

      lag -= kTickTime;
      time_ += kTickTime;
      ++ticks;
    }

    ++frames;
    if (!minimized_) {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      gui(delta);

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    window_->on_update();

    if (second > 1s) {
      uint16_t seconds = static_cast<uint16_t>(std::chrono::duration_cast<std::chrono::seconds>(second).count());

      fps_   = static_cast<uint16_t>(frames / seconds);
      tps_   = static_cast<uint16_t>(ticks / seconds);
      frames = 0;
      ticks  = 0;
      second = 0ns;
    }
  }
}

void Resin::update(duration_t delta) {
  const float seconds_dt = std ::chrono::duration_cast<std::chrono::duration<float>>(delta).count();

  directional_light_->transform.rotate(glm::angleAxis(std::chrono::duration<float>(delta).count(), glm::vec3(0, 1, 0)));

  if (sdf_tree_.is_dirty()) {
    shader_->fragment_shader().set_ext_defi("SDF_CODE", sdf_tree_.gen_shader_code());
    Logger::debug("{}", sdf_tree_.gen_shader_code());
    shader_->recompile();
    setup_shader_uniforms();
    Logger::info("Refreshed the SDF Tree");
    sdf_tree_.mark_clean();
  }

  primitive_ubo_->bind();
  primitive_ubo_->update_dirty(sdf_tree_);
  primitive_ubo_->unbind();

  // TODO(SDF-87)
  shader_->set_uniform("u_sdf_materials[0]", *sphere_mat_);
  shader_->set_uniform("u_sdf_materials[1]", *cube_mat_);

  shader_->set_uniform("u_dirLight", *directional_light_);
  shader_->set_uniform("u_pointLight", *point_light_);

  update_camera_operators(seconds_dt);
  update_camera_distance();
  interpolate(seconds_dt);

  FileDialog::instance().update();
}

void Resin::gui(duration_t delta) {
  const float seconds_dt = std ::chrono::duration_cast<std::chrono::duration<float>>(delta).count();
  ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

  bool resized = false;

  if (ImGui::resin::Viewport(*framebuffer_, resized)) {
    // These click checks allow for instant camera manipulation when viewport is not focused
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered() && !ImGui::IsWindowFocused()) {
      ImGui::SetWindowFocus();
      update_vieport_active(true);
      activate_first_person_camera(window_->mouse_pos());
    }
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) && ImGui::IsWindowHovered() && !ImGui::IsWindowFocused()) {
      ImGui::SetWindowFocus();
      update_vieport_active(true);
      activate_orbiting_camera(window_->mouse_pos());
    }

    if (ImGui::BeginMenuBar()) {
      ImGui::Text("Local Transform:");
      ImGui::Checkbox("##LocalCheckbox", &use_local_gizmos_);

      static constexpr resin::StringEnumMapper<ImGui::resin::gizmo::Operation> kOps({
          {ImGui::resin::gizmo::Operation::Translation, "Translation"},  //
          {ImGui::resin::gizmo::Operation::Rotation, "Rotation"},        //
          {ImGui::resin::gizmo::Operation::Scale, "Scale"}               //
      });

      ImGui::Text("Operation:");
      ImGui::SetNextItemWidth(ImGui::CalcTextSize("Translation").x + 12.0F);
      if (ImGui::BeginCombo("##OperationCombo", kOps[gizmo_operation_].data(), ImGuiComboFlags_NoArrowButton)) {
        for (const auto [current_op, name] : kOps) {
          if (ImGui::Selectable(kOps[current_op].data())) {
            gizmo_operation_ = current_op;
          }
        }
        ImGui::EndCombo();
      }

      ImGui::EndMenuBar();
    }

    update_vieport_active(ImGui::IsWindowFocused());

    auto width  = static_cast<float>(framebuffer_->width());
    auto height = static_cast<float>(framebuffer_->height());

    // TODO(anyone) https://stackoverflow.com/questions/73601927/implicit-vector-conversion-in-imgui-imvec-glmvec
    ImVec2 pos      = ImGui::GetCursorScreenPos();
    viewport_pos_.x = pos.x;
    viewport_pos_.y = pos.y;

    if (resized) {
      camera_->set_aspect_ratio(width / height);
      shader_->set_uniform("u_resolution", glm::vec2(width, height));
      shader_->set_uniform("u_camSize", camera_->height());
    }

    framebuffer_->bind();
    render();
    framebuffer_->unbind();
    glViewport(0, 0, static_cast<GLint>(window_->dimensions().x), static_cast<GLint>(window_->dimensions().y));

    ImGui::Image((ImTextureID)(intptr_t)framebuffer_->color_texture(), ImVec2(width, height), ImVec2(0, 1),  // NOLINT
                 ImVec2(1, 0));

    if (ImGui::IsItemHovered()) {
      ImGui::SetNextFrameWantCaptureMouse(false);
    }

    ImGui::resin::gizmo::BeginFrame();
    draw_transform_gizmo();
    draw_camera_gizmo(seconds_dt);
  }

  ImGui::End();

  ImGui::SetNextWindowSizeConstraints(ImVec2(280.F, 200.F), ImVec2(FLT_MAX, FLT_MAX));
  if (ImGui::Begin("SDF Tree")) {
    selected_node_ = ImGui::resin::SDFTreeView(sdf_tree_, selected_node_);
  }
  ImGui::End();

  if (ImGui::Begin("[TEMP] Tools")) {
    float fov = camera_->fov();
    if (ImGui::DragFloat("Camera FOV", &fov, 0.5F, 10.0F, 140.0F, "%.2f")) {
      camera_->set_fov(fov);
      shader_->set_uniform("u_camSize", camera_->height());
    }
    ImGui::Text("First Person Camera:");
    bool use_local_up = first_person_camera_operator_.is_using_local_axises();
    ImGui::Checkbox("Use local axises", &use_local_up);
    first_person_camera_operator_.set_use_local_axises(use_local_up);
  }
  ImGui::End();

  ImGui::SetNextWindowSizeConstraints(ImVec2(350.F, 200.F), ImVec2(FLT_MAX, FLT_MAX));
  ImGui::Begin("[TEMP] Lights");
  if (ImGui::BeginTabBar("LightsTabBar", ImGuiTabBarFlags_None)) {
    // TODO(SDF-88): i don't want to design GUI please save me guys 🤲🙏
    if (ImGui::BeginTabItem("DirLight")) {
      ImGui::ColorEdit3("Light color", glm::value_ptr(directional_light_->color));
      ImGui::resin::TransformEdit(&directional_light_->transform);
      ImGui::DragFloat("Ambient impact", &directional_light_->ambient_impact, 0.01F, 0.0F, 2.0F, "%.2f");

      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("PointLight")) {
      ImGui::ColorEdit3("Light color", glm::value_ptr(point_light_->color));
      ImGui::resin::TransformEdit(&point_light_->transform);
      if (ImGui::TreeNode("Attenuation")) {
        ImGui::DragFloat("Constant", &point_light_->attenuation.constant, 0.01F, 0.0F, 2.0F, "%.2f");
        ImGui::DragFloat("Linear", &point_light_->attenuation.linear, 0.01F, 0.0F, 2.0F, "%.2f");
        ImGui::DragFloat("Quadratic", &point_light_->attenuation.quadratic, 0.01F, 0.0F, 2.0F, "%.2f");
        ImGui::TreePop();
      }
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
  ImGui::End();

  ImGui::Begin("[TEMP] Materials");
  if (ImGui::BeginTabBar("MaterialTabBar", ImGuiTabBarFlags_None)) {
    // TODO(SDF-87)
    if (ImGui::BeginTabItem("CubeMat")) {
      ImGui::ColorEdit3("Color", glm::value_ptr(cube_mat_->albedo));
      ImGui::DragFloat("Ambient", &cube_mat_->ambientFactor, 0.01F, 0.0F, 1.0F, "%.2f");
      ImGui::DragFloat("Diffuse", &cube_mat_->diffuseFactor, 0.01F, 0.0F, 1.0F, "%.2f");
      ImGui::DragFloat("Specular", &cube_mat_->specularFactor, 0.01F, 0.0F, 1.0F, "%.2f");
      ImGui::DragFloat("Exponent", &cube_mat_->specularExponent, 0.1F, 0.0F, 100.0F, "%.1f");
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("SphereMat")) {
      ImGui::ColorEdit3("Color", glm::value_ptr(sphere_mat_->albedo));
      ImGui::DragFloat("Ambient", &sphere_mat_->ambientFactor, 0.01F, 0.0F, 1.0F, "%.2f");
      ImGui::DragFloat("Diffuse", &sphere_mat_->diffuseFactor, 0.01F, 0.0F, 1.0F, "%.2f");
      ImGui::DragFloat("Specular", &sphere_mat_->specularFactor, 0.01F, 0.0F, 1.0F, "%.2f");
      ImGui::DragFloat("Exponent", &sphere_mat_->specularExponent, 0.1F, 0.0F, 100.0F, "%.1f");
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
  ImGui::End();

  ImGui::SetNextWindowSizeConstraints(ImVec2(350.F, 200.F), ImVec2(FLT_MAX, FLT_MAX));
  ImGui::Begin("Selection");
  if (selected_node_.has_value() && !selected_node_->expired()) {
    ImGui::resin::NodeEdit(sdf_tree_.node(*selected_node_));
  }
  ImGui::End();

#ifndef NDEBUG
  if (ImGui::Begin("DEBUG")) {
    ImGui::Text("FPS: %d", fps_);
    ImGui::Text("TPS: %d", tps_);
    ImGui::Text("Running for: %lld", std::chrono::duration_cast<std::chrono::seconds>(time_));  // NOLINT
  }
  ImGui::End();
#endif
}

void Resin::render() {
  glBindVertexArray(vertex_array_);
  shader_->bind();
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  shader_->unbind();
}

bool Resin::on_window_close(WindowCloseEvent&) {
  running_ = false;
  return true;
}

bool Resin::on_window_resize(WindowResizeEvent& e) {
  Logger::debug("Handling resize: {}!", e);
  if (e.width() == 0 || e.height() == 0) {
    minimized_ = true;
    return true;
  }

  minimized_ = false;
  glViewport(0, 0, static_cast<GLint>(e.width()), static_cast<GLint>(e.height()));
  auto width     = static_cast<float>(e.width());
  auto height    = static_cast<float>(e.height());
  ImGuiIO& io    = ImGui::GetIO();
  io.DisplaySize = ImVec2(width, height);
  return false;
}

bool Resin::on_mouse_btn_pressed(MouseButtonPressedEvent& e) {
  ImGuiIO& io = ImGui::GetIO();
  if (io.WantCaptureMouse) {
    return true;
  }

  if (e.button() == mouse::Code::MouseButtonMiddle) {
    return activate_orbiting_camera(e.pos());
  }

  if (e.button() == mouse::Code::MouseButtonRight) {
    if (camera_->transform.local_up().y < 0.0F) {
      return start_interpolation();
    }

    return activate_first_person_camera(e.pos());
  }

  glm::vec2 relative_pos = e.pos() - viewport_pos_;
  if (relative_pos.x < 0 || relative_pos.y < 0 || relative_pos.x > static_cast<float>(framebuffer_->width()) ||
      relative_pos.y > static_cast<float>(framebuffer_->height())) {
    return false;
  }

  if (e.button() == mouse::Code::MouseButtonLeft) {
    return select_node(relative_pos);
  }

  return false;
}

bool Resin::on_mouse_btn_released(MouseButtonReleasedEvent& e) {
  if (e.button() == mouse::Code::MouseButtonMiddle) {
    return deactivate_orbiting_camera();
  }

  if (e.button() == mouse::Code::MouseButtonRight) {
    return deactivate_first_person_camera();
  }

  return false;
}

bool Resin::on_key_pressed(KeyPressedEvent& e) {
  if (e.key_code() == key::Code::C) {
    return switch_ortho();
  }

  return start_moving_first_person_camera(e.key_code());
}

bool Resin::on_key_released(KeyReleasedEvent& e) { return stop_moving_first_person_camera(e.key_code()); }

bool Resin::on_scroll(ScrollEvent& e) { return zoom_camera(e.offset()); }

bool Resin::update_vieport_active(bool is_viewport_focused) {
  if (current_vieport_state_ == ViewportState::InactiveIdle && is_viewport_focused) {
    current_vieport_state_ = ViewportState::ActiveIdle;
    return true;
  }
  if (current_vieport_state_ != ViewportState::InactiveIdle && !is_viewport_focused) {
    current_vieport_state_ = ViewportState::InactiveIdle;
    return true;
  }
  return false;
}

bool Resin::draw_transform_gizmo() {
  if (selected_node_ && !selected_node_->expired()) {
    bool disabled = current_vieport_state_ != ViewportState::ActiveIdle &&
                    current_vieport_state_ != ViewportState::InactiveIdle &&
                    current_vieport_state_ != ViewportState::CameraInterpolation &&
                    current_vieport_state_ != ViewportState::GizmoTransform;

    auto& node = sdf_tree_.node(*selected_node_);
    if (ImGui::resin::gizmo::Transform(
            node.transform(), *camera_,
            use_local_gizmos_ ? ImGui::resin::gizmo::Mode::Local : ImGui::resin::gizmo::Mode::World, gizmo_operation_,
            disabled)) {
      node.mark_dirty();
      current_vieport_state_ = ViewportState::GizmoTransform;
      return true;
    }
  }

  if (current_vieport_state_ == ViewportState::GizmoTransform && !ImGui::resin::gizmo::IsTransformUsed()) {
    current_vieport_state_ = ViewportState::ActiveIdle;
    return true;
  }

  return false;
}

bool Resin::draw_camera_gizmo(float dt) {
  bool disabled = current_vieport_state_ != ViewportState::ActiveIdle &&
                  current_vieport_state_ != ViewportState::InactiveIdle &&
                  current_vieport_state_ != ViewportState::CameraInterpolation &&
                  current_vieport_state_ != ViewportState::GizmoCamera;

  if (ImGui::resin::gizmo::CameraView(*camera_, camera_distance_, dt, disabled)) {
    shader_->set_uniform("u_iV", camera_->inverse_view_matrix());
    current_vieport_state_ = ViewportState::GizmoCamera;
    return true;
  }

  if (current_vieport_state_ == ViewportState::GizmoCamera) {
    current_vieport_state_ = ViewportState::ActiveIdle;
    return true;
  }

  return false;
}

bool Resin::update_camera_distance() {
  if (current_vieport_state_ != ViewportState::GizmoCamera &&
      current_vieport_state_ != ViewportState::CameraInterpolation) {
    camera_distance_ = glm::length(camera_->transform.pos());
    return true;
  }
  return false;
}

bool Resin::switch_ortho() {
  if (current_vieport_state_ != ViewportState::InactiveIdle &&
      current_vieport_state_ != ViewportState::CameraInterpolation) {
    camera_->set_orthographic(!camera_->is_orthographic());
    shader_->set_uniform("u_ortho", camera_->is_orthographic());
    shader_->set_uniform("u_camSize", camera_->height());
    return true;
  }

  return false;
}

bool Resin::activate_first_person_camera(glm::vec2 mouse_pos) {
  if (current_vieport_state_ == ViewportState::ActiveIdle) {
    current_vieport_state_ = ViewportState::FirstPersonCamera;
    window_->set_mouse_cursor_mode(mouse::CursorMode::Disabled);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
    first_person_camera_operator_.start(mouse_pos);
    return true;
  }

  return false;
}

bool Resin::deactivate_first_person_camera() {
  if (current_vieport_state_ == ViewportState::FirstPersonCamera) {
    current_vieport_state_ = ViewportState::ActiveIdle;
    window_->set_mouse_cursor_mode(mouse::CursorMode::Normal);
    ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
    first_person_camera_operator_.stop();
    return true;
  }

  return false;
}

bool Resin::start_moving_first_person_camera(key::Code key_code) {
  if (current_vieport_state_ == ViewportState::FirstPersonCamera) {
    first_person_camera_operator_.start_move(key_code);
    return true;
  }
  return false;
}

bool Resin::stop_moving_first_person_camera(key::Code key_code) {
  first_person_camera_operator_.stop_move(key_code);
  return true;
}

bool Resin::activate_orbiting_camera(glm::vec2 mouse_pos) {
  if (current_vieport_state_ == ViewportState::ActiveIdle) {
    current_vieport_state_ = ViewportState::OrbitingCamera;

    auto dir = glm::normalize(camera_->transform.pos());
    if (camera_->transform.local_up().y > 0.0F) {
      camera_->transform.set_local_rot(glm::quatLookAt(-dir, glm::vec3(0.0F, 1.0F, 0.0F)));
    } else {
      camera_->transform.set_local_rot(glm::quatLookAt(-dir, glm::vec3(0.0F, -1.0F, 0.0F)));
    }

    window_->set_mouse_cursor_mode(mouse::CursorMode::Disabled);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
    orbiting_camera_operator_.start(mouse_pos);
    return true;
  }
  return false;
}

bool Resin::deactivate_orbiting_camera() {
  if (current_vieport_state_ == ViewportState::OrbitingCamera) {
    current_vieport_state_ = ViewportState::ActiveIdle;
    window_->set_mouse_cursor_mode(mouse::CursorMode::Normal);
    ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
    orbiting_camera_operator_.stop();
    return true;
  }
  return false;
}

bool Resin::update_camera_operators(float dt) {
  if (current_vieport_state_ == ViewportState::OrbitingCamera) {
    if (orbiting_camera_operator_.update(*camera_, camera_distance_, window_->mouse_pos(), dt)) {
      shader_->set_uniform("u_iV", camera_->inverse_view_matrix());
      shader_->set_uniform("u_camSize", camera_->height());
      return true;
    }
  } else if (current_vieport_state_ == ViewportState::FirstPersonCamera) {
    if (first_person_camera_operator_.update(*camera_, window_->mouse_pos(), dt)) {
      shader_->set_uniform("u_iV", camera_->inverse_view_matrix());
      shader_->set_uniform("u_camSize", camera_->height());
      return true;
    }
  }

  return false;
}

bool Resin::zoom_camera(glm::vec2 offset) {
  if (current_vieport_state_ == ViewportState::InactiveIdle ||
      current_vieport_state_ == ViewportState::FirstPersonCamera ||
      current_vieport_state_ == ViewportState::OrbitingCamera ||
      current_vieport_state_ == ViewportState::CameraInterpolation) {
    return false;
  }

  if (std::abs(offset.y) > 0.0F) {
    camera_distance_ = glm::length(camera_->transform.local_pos());
    camera_distance_ -= offset.y * 0.8F;
    if (camera_distance_ < 0.0F) {
      camera_distance_ = 0.0F;
    }

    auto dir = glm::normalize(camera_->transform.local_pos());
    camera_->transform.set_local_pos(dir * camera_distance_);
    if (camera_->transform.local_up().y > 0.0F) {
      camera_->transform.set_local_rot(glm::quatLookAt(-dir, glm::vec3(0.0F, 1.0F, 0.0F)));
    } else {
      camera_->transform.set_local_rot(glm::quatLookAt(-dir, glm::vec3(0.0F, -1.0F, 0.0F)));
    }

    shader_->set_uniform("u_iV", camera_->inverse_view_matrix());
    if (camera_->is_orthographic()) {
      camera_->recalculate_projection();
      shader_->set_uniform("u_camSize", camera_->height());
    }

    return true;
  }

  return false;
}

bool Resin::select_node(glm::vec2 relative_pos) {
  if (current_vieport_state_ == ViewportState::ActiveIdle || current_vieport_state_ == ViewportState::InactiveIdle) {
    framebuffer_->bind();
    int id = framebuffer_->sample_mouse_pick(static_cast<size_t>(relative_pos.x), static_cast<size_t>(relative_pos.y));
    framebuffer_->unbind();

    selected_node_ = id == -1 ? std::nullopt : sdf_tree_.get_view_from_raw_id(static_cast<size_t>(id));
    return true;
  }

  return false;
}

bool Resin::start_interpolation() {
  if (current_vieport_state_ == ViewportState::ActiveIdle || current_vieport_state_ == ViewportState::InactiveIdle) {
    current_vieport_state_ = ViewportState::CameraInterpolation;
  }

  return true;
}

bool Resin::interpolate(float dt) {
  static const float kInterpolationTime = 0.2F;

  static bool interpolation_started   = false;
  static float interpolation_progress = 0.0F;
  static glm::quat interpolation_start_quat;
  static glm::quat interpolation_end_quat;

  if (current_vieport_state_ != ViewportState::CameraInterpolation) {
    return false;
  }

  if (!interpolation_started) {
    interpolation_started    = true;
    interpolation_progress   = 0.0F;
    interpolation_start_quat = camera_->transform.local_rot();
    interpolation_end_quat   = glm::quatLookAt(camera_->transform.local_front(), glm::vec3(0.0F, 1.0F, 0.0F));
    camera_->transform.local_rot();
  }

  interpolation_progress += dt;
  if (interpolation_progress > kInterpolationTime) {
    current_vieport_state_ = ViewportState::ActiveIdle;
    interpolation_started  = false;
    return true;
  }

  float t = interpolation_progress / kInterpolationTime;
  camera_->transform.set_local_rot(glm::slerp(interpolation_start_quat, interpolation_end_quat, t));
  shader_->set_uniform("u_iV", camera_->inverse_view_matrix());
  shader_->set_uniform("u_camSize", camera_->height());

  return true;
}

}  // namespace resin
