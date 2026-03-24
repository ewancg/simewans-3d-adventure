#include "Application.h"
#include <SDL3/SDL_init.h>
#include <print>

// Local includes above, we want tests in all other implementation files to be sourced
#ifndef UNIT_TESTING

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

// The goal with the callbacks is to eliminate a global state for the runtime context of
// the application so it can defer to the OS's runtime management; allows us to not worry about
// the mainloop and allows us to receive events via. callback instead of polling for them (adding
// latency waiting for the next frame)

// NOLINTBEGIN(*-owning-memory, *-no-malloc, *-avoid-c-arrays, *-declaration-parameter-name)
// These are outright required by SDL and/or its callback system; C++ core guidelines with a C
// library is an uphill battle; to store a single gsl::owner denoting the primary app (what it
// wants) I would be completely undermining the callback system's benefits
static SDL_AppResult getApplication(void *t_inState, Application **t_outApp) {
  const static auto badState = [] {
    std::println(stderr, "App state pointer was invalidated since last access.");
    return SDL_APP_FAILURE;
  };
  if (t_inState == nullptr) {
    return badState();
  }
  *t_outApp = static_cast<Application *>(t_inState);
  if (t_outApp == nullptr) {
    return badState();
  }
  return SDL_APP_CONTINUE;
};

// NOLINTNEXTLINE(*-identifier-naming)
SDL_AppResult SDL_AppInit(void **t_appState, int argc, char *argv[]) {
  // TODO: process command line arguments
  (void)argc;
  (void)argv;

  // If we were not using SDL callbacks I would keep Application on the stack
  // If the Application object ever overflows past 1 page's length, this is completely useless
  auto *stateBuf = std::aligned_alloc(std::max(sizeof(Application), Application::getHostPageSize()),
                                      sizeof(Application));

  auto *app = new (stateBuf) Application{};
  if (auto err = app->init(); err) {
    std::println(stderr, "Fatal error during app initialization: {}", err.string());
    return SDL_APP_FAILURE;
  }
  *t_appState = app;

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *t_appState) {
  Application *app{};
  if (auto err = getApplication(t_appState, &app); err != SDL_APP_CONTINUE) {
    return err;
  }
  static bool ticking{};
  if (auto err = app->isTicking(ticking); err) {
    std::println(stderr, "Couldn't query app's ticking state ({})", err.string());
    return SDL_APP_FAILURE;
  }
  if (app->isInitialized() && !ticking) {
    return SDL_APP_SUCCESS;
  }
  if (auto err = app->update(); err) {
    std::println(stderr, "Fatal error during execution ({})", err.string());
    return SDL_APP_FAILURE;
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *t_appState, SDL_Event *t_evt) {
  // TODO: rig up to inputs (this is not guaranteed to be called on the main thread so we need to
  // take care)
  Application *app{};
  if (auto err = getApplication(t_appState, &app); err != SDL_APP_CONTINUE) {
    return err;
  }
  auto evt = Event(t_evt);
  if (auto err = app->onEvent(evt); err) {
    std::println(stderr, "Fatal error processing events ({})", err.string());
  }
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *t_appState, SDL_AppResult t_result) {
  t_result = SDL_APP_FAILURE;
  Application *app{};
  if (auto err = getApplication(t_appState, &app); err != SDL_APP_CONTINUE) {
    t_result = err;
    return;
  }
  if (auto err = app->destroy(); !err) {
    std::println(stderr, "Fatal error during shutdown {}", err.string());
    t_result = SDL_APP_SUCCESS;
  }
  app->~Application();
  std::free(t_appState);
}

// NOLINTEND(*-owning-memory, *-no-malloc, *-avoid-c-arrays, *-declaration-parameter-name)
#endif
