#include "graphics.h"
#include "input.h"
#ifndef UNIT_TESTING

#define TICK_DIVISOR 1000000000.0
#define TICK_INTERVAL_60FPS 16666667 // 16666667 is the nanoseconds per frame at 60 fps

// Start and End Frame Functions
static void start_frame(Window &window, Input &input) {
  const auto error = input::update(input);
  if (error) {
    auto [type, msg] = error.value();
    std::println(stderr, "Error while receiving new input: {}", msg);
  }
}

static float end_frame(uint64_t framewait, Window &window) {
  static uint64_t past;
  // screenUpdate(screen);

  uint64_t now = SDL_GetTicksNS();
  uint64_t frametime = now - past;
  // printf("%u\n", frametime.count());

  SDL_DelayPrecise(framewait - frametime);

  past = now;
  return static_cast<float>(framewait) / TICK_DIVISOR;
}

int main(int argc, char *argv[]) {
  auto ok = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

  auto window = Window{.m_name = "AwesomeSauce"};
  auto input = Input{.m_window = window}; // This is a reference

  window::init(window);
  input::init(input);

  window::show(window);

  while (window.m_ticking) {
    start_frame(window, input);
    end_frame(TICK_INTERVAL_60FPS, window);
  }
}

#else
#include <catch2/catch_test_macros.hpp>
// TEST_CASE("unit test", "") { REQUIRE(true); }
#endif
