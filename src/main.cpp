#include <SDL3/SDL_events.h>
#include <functional>

#ifndef UNIT_TESTING
#ifdef ENABLE_APP_CALLBACKS
#include "Application/SDLCallbacks.h"
#else

#include "Application.h"
#include "Application/Config.h"

struct Configs {
  MainConfigData     main;
  GraphicsConfigData graphics;
};

// NOLINTBEGIN(*-avoid-non-const-global-variables)
namespace {
std::atomic_bool ticking = true;
Application      app{}; // NOLINT(cert-err58-cpp)
Configs         *config;
} // namespace
// NOLINTEND(*-avoid-non-const-global-variables)

template <typename Stage> static constexpr auto mkErrorFn(Stage t_stage) {
  return [t_stage](auto t_err) {
    ticking = false;
    std::println(stderr, "Fatal error during {}: {}", t_stage, t_err.string());
  };
};

/// Returns success
static inline bool processEvent(SDL_Event *t_event) {
  auto evt = Event(t_event);
  app.onEvent(evt).mapError(mkErrorFn("event processing"));
  return ticking;
}

static inline void update() {
  app.update().mapError(mkErrorFn("execution"));
  // frame delay/count here
}

/// Event poll + callback sequence when:
///  - `ENABLE_APP_CALLBACKS` is not defined
///  - `main.force_synchronous_events` is enabled
///  - `graphics.fps_cap` is disabled
static inline void synchronousEventsUncapped(SDL_Event *t_event) {
  while (SDL_PollEvent(t_event)) {
    processEvent(t_event);
  }
  update();
}

static inline void waitForFrameEnd(uint64_t t_endNs) {
  auto now = SDL_GetTicksNS();
  if (now < t_endNs) {
    SDL_DelayPrecise(t_endNs - now);
  }
}

/// Event poll + callback sequence when:
///  - `ENABLE_APP_CALLBACKS` is not defined
///  - `main.force_synchronous_events` is enabled
///  - `graphics.fps_cap` is enabled
static inline void synchronousEventsOnFrameInterval(SDL_Event *t_event, auto t_fps) {
  uint64_t targetNs    = FPS_TO_NS(t_fps);
  auto     endNs       = SDL_GetTicksNS() + targetNs;
  auto     remainingNs = (SDL_GetTicksNS() < endNs) ? (endNs - SDL_GetTicksNS()) : 0;
  if (SDL_WaitEventTimeout(t_event, static_cast<int32_t>(remainingNs / NS_IN_MS))) {
    processEvent(t_event);
  }
  synchronousEventsUncapped(t_event);
  while (SDL_PollEvent(t_event)) {
    processEvent(t_event);
  }
  update();
  waitForFrameEnd(endNs);
}

int main() {
  app.init().mapError(mkErrorFn("initialization"));

  using enum Config::ESystemConfigs;
  *config   = Configs{.main     = std::any_cast<MainConfigData>(app.config.get(MAIN)),
                      .graphics = std::any_cast<GraphicsConfigData>(app.config.get(GRAPHICS))};
  auto &fps = config->graphics.fps_cap;

  if (config->main.force_synchronous_events) {
    // Event is stored locally
    SDL_Event evt{};

    while (ticking) {
      if (fps > 0) {
        synchronousEventsOnFrameInterval(&evt, fps);
      }
      update();
    }
  } else {
    // Potentially called by another thread
    SDL_AddEventWatch(
        [](void *, SDL_Event *t_event) {
          processEvent(t_event);
          return true;
        },
        nullptr);

    while (ticking) {
      if (fps > 0) {
        waitForFrameEnd(SDL_GetTicksNS() + uint64_t(FPS_TO_NS(fps)));
      }
      update();
    }
  }
  app.destroy().mapError(mkErrorFn("deinitialization"));
}

#endif
#endif
