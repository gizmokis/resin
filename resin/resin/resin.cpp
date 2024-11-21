#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <libresin/utils/logger.hpp>
#include <memory>
#include <random>
#include <resin/core/window.hpp>
#include <resin/event/event.hpp>
#include <resin/event/window_events.hpp>
#include <resin/resin.hpp>

#include "libresin/core/resources/shader_resource.hpp"

glm::vec3 generateRandomVec3(float min, float max) {
  // Create a random number generator
  std::random_device rd;   // Seed
  std::mt19937 gen(rd());  // Mersenne Twister generator
  std::uniform_real_distribution<float> dis(min, max);

  // Generate random x, y, z components
  float x = dis(gen);
  float y = dis(gen);
  float z = dis(gen);

  return glm::vec3(x, y, z);
}

namespace resin {

Resin::Resin() {
  dispatcher_.subscribe<WindowCloseEvent>(BIND_EVENT_METHOD(on_window_close));
  dispatcher_.subscribe<WindowResizeEvent>(BIND_EVENT_METHOD(on_window_resize));
  dispatcher_.subscribe<WindowTestEvent>(BIND_EVENT_METHOD(on_test));

  {
    WindowProperties properties;
    properties.eventDispatcher = dispatcher_;

    window_ = std::make_unique<Window>(std::move(properties));
  }

  const std::filesystem::path path = std::filesystem::current_path() / "assets";

  shader_ = std::make_unique<RenderingShaderProgram>("default", *shader_resource_manager_.get_res(path / "test.vert"),
                                                     *shader_resource_manager_.get_res(path / "test.frag"));
  positions_.emplace_back(generateRandomVec3(-2, 2));
  nodes_.emplace_back(node(-1, count_++));
  ++count2_;
  shader_->set_uniform_array("u_pos", std::span(positions_));

  float vertices[4 * (3 + 4)] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f};

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

  glGenBuffers(1, &uniform_buffer_);
  GLuint blockIndex = glGetUniformBlockIndex(shader_->id(), "NodesBlock");
  glUniformBlockBinding(shader_->id(), blockIndex, 0);  // Bind to binding point 0
  glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(node) * 200 + sizeof(int), NULL,
               GL_STATIC_DRAW);  // Allocate space for 3 mat4s (model, view, projection)
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer_);

  // Upload to UBO
  glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(node) * nodes_.size(), nodes_.data());
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(node) * 200, sizeof(int), &count2_);
}

void Resin::run() {
  using clock = std::chrono::high_resolution_clock;

  duration_t lag(0ns);
  duration_t second(0ns);
  auto previous_time = clock::now();

  uint16_t frames = 0U;
  uint16_t ticks  = 0U;

  uint8_t ctr = 0U;
  uint16_t big_frames = 0U;

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
      big_frames += frames;
      frames = 0;
      ticks  = 0;
      second = 0ns;
      ctr++;
    }

    if (ctr == 5U) {
      std::println("{},{}", count_, big_frames);
      big_frames = 0;
      ctr = 0;

      WindowTestEvent e;
      on_test(e);
    }
  }
}

void Resin::update(duration_t) {
  window_->set_title(std::format("Resin [{} FPS {} TPS] running for: {}", fps_, tps_,
                                 std::chrono::duration_cast<std::chrono::seconds>(time_)));

  // shader_->set_uniform("iTime", std::chrono::duration<float>(time_).count());
}

void Resin::render() {
  glClearColor(0.1f, 0.1f, 0.1f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);

  {
    glBindVertexArray(vertex_array_);
    shader_->bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    shader_->unbind();
  }

  window_->on_update();
}

bool Resin::on_window_close(WindowCloseEvent&) {
  running_ = false;
  return true;
}

bool Resin::on_test(WindowTestEvent&) {
  positions_.emplace_back(generateRandomVec3(-2, 2));

  nodes_.emplace_back(node{-1, count_++});
  ++count2_;
  nodes_.emplace_back(node{0, 0});
  ++count2_;

  shader_->set_uniform_array("u_pos", std::span(positions_));
  glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(node) * nodes_.size(), nodes_.data());
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(node) * 200, sizeof(int), &count2_);
  return true;
}

bool Resin::on_window_resize(WindowResizeEvent& e) {
  Logger::debug("Handling resize: {}!", e);
  if (e.width() == 0 || e.height() == 0) {
    minimized_ = true;
    return true;
  }

  minimized_ = false;
  // TODO(SDF-28): set viewport
  return false;
}

}  // namespace resin
