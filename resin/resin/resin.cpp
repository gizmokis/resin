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
#include <libresin/utils/logger.hpp>
#include <memory>
#include <resin/components/sdf_tree.hpp>
#include <resin/core/window.hpp>
#include <resin/event/event.hpp>
#include <resin/event/window_events.hpp>
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

  // transform_ = std::make_unique<Transform>();
  cube_transform_.set_local_pos(glm::vec3(1, 1, 0));
  camera_       = std::make_unique<Camera>(true, 90.F, 16.F / 9.F, 0.75F, 100.F);
  glm::vec3 pos = glm::vec3(0, 2, 3);
  camera_->transform.set_local_pos(pos);
  glm::vec3 direction = glm::normalize(-pos);
  camera_->transform.set_local_rot(glm::quatLookAt(direction, glm::vec3(0, 1, 0)));
  camera_->transform.set_parent(camera_rig_);
  shader_ = std::make_unique<RenderingShaderProgram>("default", *shader_resource_manager_.get_res(path / "test.vert"),
                                                     *shader_resource_manager_.get_res(path / "test.frag"));

  // TODO(anyone): temporary? at least error-check
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
  sdf_tree_.root->push_child<SphereNode>(SDFBinaryOperation::Inter);
  sdf_tree_.root->push_child<GroupNode>(SDFBinaryOperation::Union);
  sdf_tree_.root->push_child<CubeNode>(SDFBinaryOperation::Union);
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
      render();
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

  cube_transform_.rotate(glm::angleAxis(2 * std::chrono::duration<float>(delta).count(), glm::vec3(0, 1, 0)));
  camera_rig_.rotate(glm::angleAxis(std::chrono::duration<float>(delta).count(), glm::vec3(0, 1, 0)));

  shader_->set_uniform("u_iV", camera_->inverse_view_matrix());
  shader_->set_uniform("u_resolution", glm::vec2(window_->dimensions()));
  shader_->set_uniform("u_focal", camera_->near_plane());
  shader_->set_uniform("u_iM", cube_transform_.world_to_local_matrix());
  shader_->set_uniform("u_ortho", camera_->is_orthographic);
  shader_->set_uniform("u_camSize", camera_->height());
}

void Resin::render() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ::ImGui::NewFrame();

  glClearColor(0.1F, 0.1F, 0.1F, 1.F);
  glClear(GL_COLOR_BUFFER_BIT);

  if (::ImGui::Begin("SDF Tree")) {
    resin::ImGui::SDFTreeView(sdf_tree_);
  }
  ::ImGui::ShowDemoWindow();
  ::ImGui::End();
  ::ImGui::Render();

  {
    glBindVertexArray(vertex_array_);
    shader_->bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    shader_->unbind();
  }

  ImGui_ImplOpenGL3_RenderDrawData(::ImGui::GetDrawData());

  window_->on_update();
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
  camera_->set_aspect_ratio(static_cast<float>(e.width()) / static_cast<float>(e.height()));
  return false;
}

bool Resin::on_test(WindowTestEvent&) {
  camera_->is_orthographic = !camera_->is_orthographic;

  return false;
}

}  // namespace resin
