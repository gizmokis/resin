#ifndef RESIN_KEY_EVENTS_HPP
#define RESIN_KEY_EVENTS_HPP

#include <resin/core/key_codes.hpp>
#include <resin/event/event.hpp>

namespace resin {

class KeyPressedEvent : public Event<EventType::KeyPressedEvent> {
 public:
  EVENT_NAME(KeyPressedEvent)

  explicit KeyPressedEvent(key::Code key_code) : key_code_(key_code) {}

  key::Code key_code() const { return key_code_; }

  std::string to_string() const override { return std::format("{}: {}", name(), key::kKeyCodeNames[key_code_]); }

 private:
  key::Code key_code_;
};

class KeyReleasedEvent : public Event<EventType::KeyReleasedEvent> {
 public:
  EVENT_NAME(KeyReleasedEvent)

  explicit KeyReleasedEvent(key::Code key_code) : key_code_(key_code) {}

  key::Code key_code() const { return key_code_; }

  std::string to_string() const override { return std::format("{}: {}", name(), key::kKeyCodeNames[key_code_]); }

 private:
  key::Code key_code_;
};

}  // namespace resin

#endif  // RESIN_KEY_EVENTS_HPP
