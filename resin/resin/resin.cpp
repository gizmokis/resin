#include <chrono>
#include <cstdint>
#include <format>
#include <libresin/utils/logger.hpp>
#include <memory>
#include <resin/core/window.hpp>
#include <resin/event/event.hpp>
#include <resin/event/window_events.hpp>
#include <resin/resin.hpp>

namespace resin {

Resin::Resin() {
  dispatcher_ = std::make_unique<EventDispatcher>();
  dispatcher_->subscribe<WindowCloseEvent>(BIND_EVENT_METHOD(on_window_close));
  dispatcher_->subscribe<WindowResizeEvent>(BIND_EVENT_METHOD(on_window_resize));

  {
    WindowProperties properties;
    properties.eventDispatcher = *dispatcher_;

    window_ = std::make_unique<Window>(std::move(properties));
  }
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

void Resin::update(duration_t) {
  window_->set_title(std::format("Resin [{} FPS {} TPS] running for: {}", fps_, tps_,
                                 std::chrono::duration_cast<std::chrono::seconds>(time_)));
}

void Resin::render() { window_->on_update(); }

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
  // TODO(SDF-28): set viewport
  return false;
}

}  // namespace resin
