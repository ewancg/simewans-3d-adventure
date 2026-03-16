// #include "Audio.h"
// #include "Graphics.h"
// #include "Input.h"
// #include "Window.h"
// #include "common.h"
// #ifndef UNIT_TESTING
//
// #define ERROR_ENTRIES(E) \
//   E(ACCESS_WITHOUT_INIT, "executing a post-initialization operation") \
//   E(DELETE_AFTER_FRAME, "executing a frame-end deferred deletion request") \
//   DEFINE_BASE_ERROR_TYPES(Subsystem, 0, ERROR_ENTRIES);
// #undef ERROR_ENTRIES
//

#include "Application.h"
#include <SDL3/SDL_init.h>
#include <print>

// NOLINTBEGIN(*-owning-memory, *-no-malloc, *-avoid-c-arrays)
// These are outright required by SDL and/or its platform-friendly callback system
// The goal is to eliminate a global state for the runtime context of the application and defer to
// the OS for ownership semantics, so to store a single gsl::owner denoting the primary app state
// will warn just as much as using the raw pointers directly

static SDL_AppResult getApplicationFromStateHandle(void *t_inState, Application *t_outApp) {
  const static auto badState = [] {
    std::println(stderr, "App state pointer was invalidated since last frame.");
    return SDL_APP_FAILURE;
  };
  if (t_inState == nullptr) {
    return badState();
  }
  t_outApp = static_cast<Application *>(t_inState);
  if (t_outApp == nullptr) {
    return badState();
  }
  return SDL_APP_CONTINUE;
};

static SDL_AppResult SDL_AppInit(void **t_state, int t_argc, char *t_argv[]) {
  // TODO: process command line arguments
  (void)t_argc;
  (void)t_argv;

  auto app = Application{};
  if (auto err = app.init(); err) {
    std::println(stderr, "Fatal error during app initialization: while {}", err.string());
    return SDL_APP_FAILURE;
  }

  auto *stateBuf = std::aligned_alloc(alignof(Application), sizeof(Application));
  new (stateBuf) Application(std::move(app));
  *t_state = stateBuf;
}

static SDL_AppResult SDL_AppIterate(void *t_state) {
  Application *app{};
  if (auto err = getApplicationFromStateHandle(t_state, app); err != SDL_APP_CONTINUE) {
    return err;
  }
  if (auto err = app->update(); err) {
    std::println(stderr, "Fatal error while {}", err.string());
    return SDL_APP_FAILURE;
  }
}

static void SDL_AppQuit(void *t_state, SDL_AppResult t_result) {
  Application *app{};
  if (auto err = getApplicationFromStateHandle(t_state, app); err != SDL_APP_CONTINUE) {
    t_result = err;
    return;
  }
  if (auto err = app->destroy(); !err) {
    std::println(stderr, "Fatal error during shutdown: while {}", err.string());
    t_result = SDL_APP_SUCCESS;
  }
  app->~Application();
  std::free(t_state);
}
// NOLINTEND(*-owning-memory, *-no-malloc, *-avoid-c-arrays)

// int main() { std::println("{}", sizeof(ESubsystemError)); }
// int main(int argc, char *argv[]) {
//   (void)argc;
//   (void)argv;
//
//   static auto passive_error = []<typename E>(E err) {
//     std::println(stderr, "Error encountered {}", err.string());
//   };
//   static auto fatal_error = []<typename E>(E err) {
//     std::println(stderr, "Unrecoverable error encountered {}", err.string());
//     std::quick_exit(1);
//   };
//
//   auto audio = Audio();
//   auto graphics = Graphics();
//   auto window = Window();
//   auto input = Input(window);
//
//   // Now we can prepare static data like shaders, vertex buffers, textures, samplers, etc
//
//   auto subsystems = SubsystemContainer(audio, graphics, window, input);
//   SDL_GPUTexture *tex{};
//   graphics.getWindowSwapchainTexture(window, tex);
//
//   graphics.setFrameInterval(NSPF_60FPS);
//
//   window.show();
//   double last_frame_time{};
//
//   for (bool running = true; running; window.getTicking(running).mapError([&running](auto &err) {
//          passive_error(err);
//          running = false;
//        })) {
//     graphics.beginFrame(tex).mapError(passive_error);
//     subsystems.forEach([](auto &inst) { inst.update().mapError(passive_error); });
//     graphics.endFrame(last_frame_time);
//   }
// }

// #else
// #include <catch2/catch_test_macros.hpp>
//  TEST_CASE("unit test", "") { REQUIRE(true); }
// #endif
