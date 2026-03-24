#pragma once

#include "Audio.h"
#include "Graphics.h"
#include "Input.h"
#include "Subsystem.h"
#include "Window.h"
#include <functional>
#include <unistd.h> // sysconf(_SC_PAGESIZE)

#include "Application/Config.h"
#include "Application/Error.h"
#include "Application/Metadata.h"

class Application : public Subsystem<ApplicationError> {
  SUBSYSTEM(Application);

public:
  struct EventRange {
    SDL_EventType first;
    SDL_EventType last;
  };
  // NOLINTBEGIN (*-non-private-member-variables-in-classes)
  /// Public object parameters which must be populated at the callsite before initialization
  /// Subsystems (ticked every frame, can subscribe to events, can have properties)
  Audio                     audio    = Audio(*this);
  Graphics                  graphics = Graphics(*this);
  Window                    window   = Window(*this);
  Input                     input    = Input(*this, window);
  /// Non-subsystem children
  Config                    config   = Config(*this);
  constexpr static Metadata metadata{};
  // NOLINTEND

  Error onEvent(Event &);

  static uint64_t getHostPageSize() { return uint64_t(sysconf(_SC_PAGESIZE)); };

  using EventSubscriber = std::function<ApplicationError(Event &)>;
  void subscribeToEvents(EventSubscriber t_fn, SDL_EventType t_first, SDL_EventType t_last) {
    eventSubscriptions.emplace_back(std::forward<EventSubscriber>(t_fn),
                                    EventRange{.first = t_first, .last = t_last});
  };
  template <typename T>
  void subscribeToEvents(T *t_this, ApplicationError (T::*t_fn)(Event &), SDL_EventType t_first,
                         SDL_EventType t_last) {
    subscribeToEvents(std::bind(t_fn, t_this, std::placeholders::_1), t_first, t_last);
  }
  DEFINE_PROPERTY(bool, m_isTicking, isTicking, setTicking, false);

private:
  // Lazy model, has no tracking, may require change later
  std::vector<std::pair<EventSubscriber, EventRange>> eventSubscriptions;
};
