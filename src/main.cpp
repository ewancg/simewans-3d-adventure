#include "graphics.h"
#include "input.h"
#include "window.h"

#ifndef UNIT_TESTING

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  static auto passive_error = []<typename E>(E err) {
    auto [type, str] = *err;
    auto type_str = err.context();
    std::println(stderr, "Error encountered while {}: {}", type_str, str);
  };
  static auto fatal_error = []<typename E>(E err) {
    passive_error(err);
    std::quick_exit(1);
  };

  auto graphics = Graphics();
  auto window = Window();
  auto input = Input(window);

  graphics.init().map_err(fatal_error);
  window.init().map_err(fatal_error);
  input.init().map_err(fatal_error);

  graphics.attachWindow(window).map_err(fatal_error);

  // Now we can prepare static data like shaders, vertex buffers, textures, samplers, etc

  SDL_GPUTexture *tex{};
  graphics.getWindowSwapchainTexture(window, tex);

  window.show();
  double last_frame_time{};

  while (window.ticking()) {
    graphics.beginFrame(tex).map_err(passive_error);
    input.update().map_err(passive_error);
    window.update().map_err(passive_error);

    graphics.endFrame(TICK_INTERVAL_60FPS);
  }
}

#else
#include <catch2/catch_test_macros.hpp>
// TEST_CASE("unit test", "") { REQUIRE(true); }
#endif
