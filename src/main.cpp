#include "graphics.h"
#include "input.h"
#include "window.h"

#ifndef UNIT_TESTING

#define TICK_DIVISOR 1000000000.0
#define TICK_INTERVAL_60FPS 16666667 // 16666667 is the nanoseconds per frame at 60 fps

static double wait_frame_end(const uint64_t framewait, uint64_t &past) {
  // screenUpdate(screen);

  uint64_t now = SDL_GetTicksNS();
  uint64_t frametime = now - past;
  // printf("%u\n", frametime.count());

  SDL_DelayPrecise(framewait - frametime);

  past = now;
  return static_cast<double>(framewait) / TICK_DIVISOR;
}

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

  Graphics graphics{};
  Window window{.m_graphics = graphics, .m_name = "AwesomeSauce"};
  Input input{.m_window = window};

  graphics::init(graphics).map_err(fatal_error);
  window::init(window).map_err(fatal_error);
  input::init(input).map_err(fatal_error);

  graphics::attach_window(graphics, window).map_err(fatal_error);

  // Now we can prepare static data like shaders, vertex buffers, textures, samplers, etc
  window::show(window);

  uint64_t past{};
  double last_frame_time{};

  while (window.m_ticking) {
    input::update(input).map_err(passive_error);
    window::update(window).map_err(passive_error);

    last_frame_time = wait_frame_end(TICK_INTERVAL_60FPS, past);
  }
}

#else
#include <catch2/catch_test_macros.hpp>
// TEST_CASE("unit test", "") { REQUIRE(true); }
#endif
