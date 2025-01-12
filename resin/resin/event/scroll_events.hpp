#ifndef RESIN_SCROLL_EVENTS_HPP
#define RESIN_SCROLL_EVENTS_HPP

#include <glm/ext/vector_float2.hpp>
#include <resin/core/mouse_codes.hpp>
#include <resin/event/event.hpp>

namespace resin {

class ScrollEvent : public Event<EventType::ScrollEvent> {
 public:
  EVENT_NAME(ScrollEvent)

  explicit ScrollEvent(glm::vec2 offset) : offset_(offset) {}

  glm::vec2 offset() const { return offset_; }

  std::string to_string() const override { return std::format("{}: {}x{}", name(), offset_.x, offset_.y); }

 private:
  glm::vec2 offset_;
};

}  // namespace resin

#endif  // RESIN_SCROLL_EVENTS_HPP
