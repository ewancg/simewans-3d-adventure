#ifndef UNIT_TESTING

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "Application.h"

// NOLINTBEGIN(*-owning-memory, *-no-malloc, *-avoid-c-arrays)
// These are outright required by SDL and/or its
// platform-friendly callback system The goal is to eliminate a global state for the runtime context
// of the application and defer to the OS for ownership semantics, so to store a single gsl::owner
// denoting the primary app state will warn just as much as using the raw pointers directly

static SDL_AppResult getApplication(void *t_inState, Application **t_outApp) {
  const static auto badState = [] {
    std::println(stderr, "App state pointer was invalidated since last frame.");
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
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  // TODO: process command line arguments
  (void)argc;
  (void)argv;

  // If we were not using SDL callbacks I would keep Application on the stack
  // Since the class is predictably sized, it gives us a discrete page to use
  auto *stateBuf = std::aligned_alloc(
      std::max(sizeof(Application), static_cast<uint64_t>(sysconf(_SC_PAGESIZE))),
      sizeof(Application));

  auto *app = new (stateBuf) Application();
  if (auto err = app->init(); err) {
    std::println(stderr, "Fatal error during app initialization: {}", err.string());
    return SDL_APP_FAILURE;
  }
  *appstate = app;

  return SDL_APP_CONTINUE;
}

// NOLINTNEXTLINE(*-identifier-naming)
SDL_AppResult SDL_AppIterate(void *appstate) {
  Application *app{};
  if (auto err = getApplication(appstate, &app); err != SDL_APP_CONTINUE) {
    return err;
  }
  if (auto err = app->update(); err) {
    std::println(stderr, "Fatal error ({})", err.string());
    return SDL_APP_FAILURE;
  }
  return SDL_APP_CONTINUE;
}

// NOLINTNEXTLINE(*-identifier-naming)
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  // TODO: rig up to inputs (this is not guaranteed to be called on the main thread so we would need
  // to take care)
  (void)event;

  Application *app{};
  if (auto err = getApplication(appstate, &app); err != SDL_APP_CONTINUE) {
    return err;
  }
  return SDL_APP_CONTINUE;
}

// NOLINTNEXTLINE(*-identifier-naming)
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  result = SDL_APP_FAILURE;
  Application *app{};
  if (auto err = getApplication(appstate, &app); err != SDL_APP_CONTINUE) {
    result = err;
    return;
  }
  if (auto err = app->destroy(); !err) {
    std::println(stderr, "Fatal error during shutdown {}", err.string());
    result = SDL_APP_SUCCESS;
  }
  app->~Application();
  std::free(appstate);
}
// NOLINTEND(*-owning-memory, *-no-malloc, *-avoid-c-arrays)
#else
#include <catch2/catch_test_macros.hpp>
TEST_CASE("unit test", "") { REQUIRE(true); }
#endif
