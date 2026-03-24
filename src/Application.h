#pragma once

#include "Audio.h"
#include "Graphics.h"
#include "Input.h"
#include "Subsystem.h"
#include "Window.h"
#include <functional>
#include <unistd.h> // sysconf(_SC_PAGESIZE)

#define ERROR_ENTRIES(E)                                                                           \
  E(INIT, "initializing the application")                                                          \
  E(DESTROY, "destroying the application")                                                         \
  E(CONFIG_INIT, "getting the configuration path")                                                 \
  E(CONFIG_READ, "loading configurations from disk")                                               \
  E(CONFIG_SERIALIZE, "converting an in-memory data type to string")                               \
  E(CONFIG_DESERIALIZE, "converting a string to an in-memory data type")                           \
  DEFINE_DERIVED_ERROR_TYPES(Application, Subsystem, ERROR_ENTRIES);
#undef ERROR_ENTRIES

class Application : public Subsystem<ApplicationError> {
  SUBSYSTEM(Application);

public:
  struct Metadata {
#define FIELD(NAME) const char *NAME
    FIELD(name) = "3d-adventure";
    FIELD(version) = "1.0";
    FIELD(identifier) = "com.simewan.3d-adventure";
    FIELD(author) = "simewan";
    FIELD(copyright) = "sim & ewan";
    FIELD(url) = "";
    FIELD(type) = "game";
#undef FIELD
  };
  struct EventRange {
    SDL_EventType first;
    SDL_EventType last;
  };
  // NOLINTBEGIN (*-non-private-member-variables-in-classes)
  // Object parameters must be populated at the callsite before initialization
  std::unique_ptr<Config> config;
  Audio audio = Audio(*this);
  Graphics graphics = Graphics(*this);
  Window window = Window(*this);
  Input input = Input(*this, window);
  // NOLINTEND

  Error onEvent(Event &);

  static uint64_t getHostPageSize() { return uint64_t(sysconf(_SC_PAGESIZE)); };
  const Metadata &getMetadata() { return metadata; }

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
  const Metadata metadata;

  // Lazy model, has no tracking, may require change later
  std::vector<std::pair<EventSubscriber, EventRange>> eventSubscriptions;
};
