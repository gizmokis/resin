#ifndef RESIN_MOUSE_EVENTS_HPP
#define RESIN_MOUSE_EVENTS_HPP

#include <glm/ext/vector_float2.hpp>
#include <resin/core/mouse_codes.hpp>
#include <resin/event/event.hpp>

namespace resin {

class MouseButtonPressedEvent : public Event<EventType::MouseButtonPressedEvent> {
 public:
  EVENT_NAME(MouseButtonPressedEvent)

  MouseButtonPressedEvent(mouse::Code button, glm::vec2 pos) : button_(button), pos_(pos) {}

  mouse::Code button() const { return button_; }
  glm::vec2 pos() const { return pos_; }

  std::string to_string() const override {
    return std::format("{}: {} ({}) at {} x {}", name(), mouse::kMouseCodeNames[button_],
                       mouse::kMouseCodeGLFWMapping[button_], pos_.x, pos_.y);
  }

 private:
  mouse::Code button_;
  glm::vec2 pos_;
};

class MouseButtonReleasedEvent : public Event<EventType::MouseButtonReleasedEvent> {
 public:
  EVENT_NAME(MouseButtonReleasedEvent)

  MouseButtonReleasedEvent(mouse::Code button, glm::vec2 pos) : button_(button), pos_(pos) {}

  mouse::Code button() const { return button_; }
  glm::vec2 pos() const { return pos_; }

  std::string to_string() const override {
    return std::format("{}: {} ({}) at {} x {}", name(), mouse::kMouseCodeNames[button_],
                       mouse::kMouseCodeGLFWMapping[button_], pos_.x, pos_.y);
  }

 private:
  mouse::Code button_;
  glm::vec2 pos_;
};

class MouseScrollEvent : public Event<EventType::MouseScrollEvent> {
 public:
  EVENT_NAME(MouseScrollEvent)

  explicit MouseScrollEvent(glm::vec2 offset) : offset_(offset) {}

  glm::vec2 offset() const { return offset_; }

  std::string to_string() const override { return std::format("{}: x={}, y={}", name(), offset_.x, offset_.y); }

 private:
  glm::vec2 offset_;
};

}  // namespace resin

#endif  // RESIN_MOUSE_EVENTS_HPP
