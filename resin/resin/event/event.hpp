#ifndef RESIN_EVENT_HPP
#define RESIN_EVENT_HPP

#include <concepts>
#include <format>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

#define EVENT_NAME(eventType) \
  static constexpr std::string_view name() { return #eventType; }

#define BIND_EVENT_METHOD(fn) \
  [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace resin {

enum class EventType { None = 0, WindowCloseEvent, WindowResizeEvent };

class BaseEvent {
 public:
  BaseEvent()          = default;
  virtual ~BaseEvent() = default;

  virtual EventType event_type() const  = 0;
  virtual std::string to_string() const = 0;

  BaseEvent(const BaseEvent&)            = delete;
  BaseEvent(BaseEvent&&)                 = delete;
  BaseEvent& operator=(const BaseEvent&) = delete;
  BaseEvent& operator=(BaseEvent&&)      = delete;

  bool handled = false;
};

template <EventType Type>
class Event : public BaseEvent {
 public:
  static constexpr EventType type() { return Type; }
  constexpr EventType event_type() const override { return type(); }
};

template <typename E>
concept EventConcept = std::derived_from<E, BaseEvent> && requires(E a) {
  { E::name() } -> std::same_as<std::string_view>;
  { E::type() } -> std::same_as<EventType>;
};

template <typename T, typename E>
concept EventHandler = EventConcept<E> && requires(T t, E& e) {
  { t(e) } -> std::same_as<bool>;
};

class EventDispatcher {  // TODO(SDF-73): implement event bus and event section of app loop
 public:
  template <typename E>
  using callback_t = std::function<bool(E&)>;

  template <EventConcept E, EventHandler<E> F>
  void subscribe(const F& callback_fn) {
    const EventType type = E::type();
    subscribers_[type].push_back(CallbackWrapper<E>(callback_fn));
  }

  template <EventConcept E>
  bool dispatch(E& event) {
    const EventType type = E::type();
    for (auto& callback : subscribers_[type]) {
      event.handled |= callback(event);
      if (event.handled) {
        return true;
      }
    }
    return false;
  }

  bool dispatch(BaseEvent& event) {
    const EventType type = event.event_type();
    for (auto& callback : subscribers_[type]) {
      event.handled |= callback(event);
      if (event.handled) {
        return true;
      }
    }
    return false;
  }

 private:
  std::unordered_map<EventType, std::vector<callback_t<BaseEvent>>> subscribers_;

  template <EventConcept E>
  class CallbackWrapper {
   public:
    template <EventHandler<E> F>
    explicit CallbackWrapper(const F& callback) : callback_(callback) {}

    bool operator()(BaseEvent& event) { return callback_(static_cast<E&>(event)); }

   private:
    callback_t<E> callback_;
  };
};

}  // namespace resin

template <resin::EventConcept E>
struct std::formatter<E> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const resin::BaseEvent& obj, FormatContext& ctx) const {
    return std::format_to(ctx.out(), "{}", obj.to_string());
  }
};

#endif  // RESIN_EVENT_HPP
