#ifndef RESIN_WINDOW_EVENTS_HPP
#define RESIN_WINDOW_EVENTS_HPP

#include <resin/event/event.hpp>

namespace resin {

class WindowCloseEvent : public Event<EventType::WindowCloseEvent> {
 public:
  EVENT_NAME(WindowCloseEvent)

  std::string to_string() const override { return std::string(name()); }

  WindowCloseEvent() = default;
};

class WindowTestEvent : public Event<EventType::WindowTestEvent> {
 public:
  EVENT_NAME(WindowTestEvent)

  std::string to_string() const override { return std::string(name()); }

  WindowTestEvent() = default;
};

class WindowResizeEvent : public Event<EventType::WindowResizeEvent> {
 public:
  EVENT_NAME(WindowResizeEvent)

  WindowResizeEvent(unsigned int width, unsigned int height) : width_(width), height_(height) {}

  unsigned int width() const { return width_; }
  unsigned int height() const { return height_; }

  std::string to_string() const override { return std::format("{}: {} x {}", name(), width_, height_); }

 private:
  unsigned int width_, height_;
};

}  // namespace resin

#endif  // RESIN_WINDOW_EVENTS_HPP
