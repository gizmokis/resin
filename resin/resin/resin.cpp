#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/core/transform.hpp>
#include <libresin/core/uniform_buffer.hpp>
#include <libresin/utils/logger.hpp>
#include <memory>
#include <resin/core/window.hpp>
#include <resin/event/event.hpp>
#include <resin/event/window_events.hpp>
#include <resin/imgui/node_edit.hpp>
#include <resin/imgui/sdf_tree.hpp>
#include <resin/imgui/transform_edit.hpp>
#include <resin/imgui/viewport.hpp>
#include <resin/resin.hpp>

namespace resin {

Resin::Resin() : vertex_array_(0), vertex_buffer_(0), index_buffer_(0) {
  dispatcher_.subscribe<WindowCloseEvent>(BIND_EVENT_METHOD(on_window_close));
  dispatcher_.subscribe<WindowResizeEvent>(BIND_EVENT_METHOD(on_window_resize));
  dispatcher_.subscribe<WindowTestEvent>(BIND_EVENT_METHOD(on_test));

  {
    WindowProperties properties;
    properties.eventDispatcher = dispatcher_;

    window_ = std::make_unique<Window>(std::move(properties));
  }

  const std::filesystem::path path = std::filesystem::current_path() / "assets";

  cube_mat_   = std::make_unique<Material>(glm::vec3(0.96F, 0.25F, 0.25F));
  sphere_mat_ = std::make_unique<Material>(glm::vec3(0.25F, 0.25F, 0.96F));

  camera_       = std::make_unique<Camera>(false, 70.F, 16.F / 9.F, 0.75F, 100.F);
  glm::vec3 pos = glm::vec3(0, 2, 3);
  camera_->transform.set_local_pos(pos);
  glm::vec3 direction = glm::normalize(-pos);
  camera_->transform.set_local_rot(glm::quatLookAt(direction, glm::vec3(0, 1, 0)));
  camera_->transform.set_parent(camera_rig_);

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
  sdf_tree_.root()
      .push_back_child<CubeNode>(SDFBinaryOperation::SmoothUnion)
      .transform()
      .set_local_pos(glm::vec3(1, 1, 0));
  sdf_tree_.root().push_back_child<SphereNode>(SDFBinaryOperation::SmoothUnion);

  // SDF Shader
  ShaderResource frag_shader = *shader_resource_manager_.get_res(path / "test.frag");

  frag_shader.set_ext_defi("SDF_CODE", sdf_tree_.gen_shader_code());
  Logger::info("{}", frag_shader.get_glsl());

  ubo_ = std::make_unique<UniformBuffer>(sdf_tree_.max_nodes_count());
  frag_shader.set_ext_defi("MAX_UBO_NODE_COUNT", std::to_string(ubo_->max_count()));
  ubo_->bind();
  ubo_->set(sdf_tree_);
  ubo_->unbind();

  shader_ = std::make_unique<RenderingShaderProgram>("default", *shader_resource_manager_.get_res(path / "test.vert"),
                                                     std::move(frag_shader));
  shader_->bind_uniform_buffer("Data", *ubo_);
  shader_->set_uniform("u_iV", camera_->inverse_view_matrix());
  shader_->set_uniform("u_resolution", glm::vec2(window_->dimensions()));
  shader_->set_uniform("u_nearPlane", camera_->near_plane());
  shader_->set_uniform("u_farPlane", camera_->far_plane());
  shader_->set_uniform("u_ortho", camera_->is_orthographic);
  shader_->set_uniform("u_camSize", camera_->height());

  framebuffer_ = std::make_unique<Framebuffer>(window_->dimensions().x, window_->dimensions().y);
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

      gui();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      window_->on_update();
    }

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
  window_->set_title(std::format("Resin [{} FPS {} TPS] running for: {}", fps_, tps_,
                                 std::chrono::duration_cast<std::chrono::seconds>(time_)));

  directional_light_->transform.rotate(glm::angleAxis(std::chrono::duration<float>(delta).count(), glm::vec3(0, 1, 0)));

  ubo_->bind();
  ubo_->update_dirty(sdf_tree_);
  ubo_->unbind();

  // TODO(SDF-87)
  shader_->set_uniform("u_cubeMat", *cube_mat_);
  shader_->set_uniform("u_sphereMat", *sphere_mat_);

  shader_->set_uniform("u_dirLight", *directional_light_);
  shader_->set_uniform("u_pointLight", *point_light_);
}

void Resin::gui() {
  ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

  bool resized = false;
  if (ImGui::resin::Viewport(*framebuffer_, resized)) {
    auto width  = static_cast<float>(framebuffer_->width());
    auto height = static_cast<float>(framebuffer_->height());

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
  }
  ImGui::End();

  ImGui::SetNextWindowSizeConstraints(ImVec2(280.F, 200.F), ImVec2(FLT_MAX, FLT_MAX));
  if (ImGui::Begin("SDF Tree")) {
    auto result    = ImGui::resin::SDFTreeView(sdf_tree_, selected_node_);
    selected_node_ = result.first;
    if (result.second) {
      shader_->fragment_shader().set_ext_defi("SDF_CODE", sdf_tree_.gen_shader_code());
      Logger::debug("{}", sdf_tree_.gen_shader_code());
      shader_->recompile();
      ubo_->bind();
      ubo_->update_dirty(sdf_tree_);
      ubo_->unbind();
      Logger::info("Refreshed the SDF Tree");
    }
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
  if (selected_node_.has_value()) {
    ImGui::resin::NodeEdit(sdf_tree_.node(*selected_node_));
  }

  ImGui::End();
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

bool Resin::on_test(WindowTestEvent&) {
  camera_->is_orthographic = !camera_->is_orthographic;
  shader_->set_uniform("u_ortho", camera_->is_orthographic);

  return false;
}

}  // namespace resin
