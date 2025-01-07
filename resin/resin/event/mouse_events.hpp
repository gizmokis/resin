#ifndef RESIN_MOUSE_EVENTS_HPP
#define RESIN_MOUSE_EVENTS_HPP

#include <glm/ext/vector_float2.hpp>
#include <resin/core/mouse_codes.hpp>
#include <resin/event/event.hpp>

namespace resin {

class MouseButtonPressedEvent : public Event<EventType::MouseButtonPressedEvent> {
 public:
  EVENT_NAME(MouseButtonPressedEvent)

  MouseButtonPressedEvent(int button, glm::vec2 pos)
      : button_(mouse::kMouseCodeGLFWMapping.from_value(button)), pos_(pos) {}

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

  MouseButtonReleasedEvent(int button, glm::vec2 pos)
      : button_(mouse::kMouseCodeGLFWMapping.from_value(button)), pos_(pos) {}

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

}  // namespace resin

#endif  // RESIN_MOUSE_EVENTS_HPP