#pragma once
#include "Audio.h"
#include "Graphics.h"
#include "Input.h"
#include "Subsystem.h"
#include "Window.h"
#include <functional>
#include <unistd.h> // sysconf(_SC_PAGESIZE)

#define ERROR_ENTRIES(E)                                                                           \
  E(INIT, "initializing the application") E(DESTROY, "destroying the application")
DEFINE_DERIVED_ERROR_TYPES(Application, Subsystem, ERROR_ENTRIES);
#undef ERROR_ENTRIES

class Application : public Subsystem<ApplicationError> {
  SUBSYSTEM(Application);

public:
  struct EventRange {
    SDL_EventType first;
    SDL_EventType last;
  };
  Error onEvent(Event &);

  // NOLINTBEGIN (*-non-private-member-variables-in-classes)
  // Object parameters must be populated at the callsite before initialization
  Audio m_audio = Audio(*this);
  Graphics m_graphics = Graphics(*this);
  Window m_window = Window(*this);
  Input m_input = Input(*this, m_window);
  // NOLINTEND

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
