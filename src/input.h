#pragma once
#include "graphics.h"
#include <SDL3/SDL_events.h>
#include <bitset>

struct Input;
namespace input {
enum EMapping : uint8_t {
  KEY_A = 0,
  KEY_B,
  KEY_C,
  KEY_D,
  KEY_E,
  KEY_F,
  KEY_G,
  KEY_H,
  KEY_I,
  KEY_J,
  KEY_K,
  KEY_L,
  KEY_M,
  KEY_N,
  KEY_O,
  KEY_P,
  KEY_Q,
  KEY_R,
  KEY_S,
  KEY_T,
  KEY_U,
  KEY_V,
  KEY_W,
  KEY_X,
  KEY_Y,
  KEY_Z,

  KEY_0, // NOLINT(misc-confusable-identifiers)
  KEY_1, // NOLINT(misc-confusable-identifiers)
  KEY_2,
  KEY_3,
  KEY_4,
  KEY_5,
  KEY_6,
  KEY_7,
  KEY_8,
  KEY_9,

  KEY_RETURN,
  KEY_SPACE,
  KEY_ESCAPE,
  KEY_LEFT,
  KEY_RIGHT,
  KEY_UP,
  KEY_DOWN,

  MOD_LSHIFT,
  MOD_LCTRL,
  MOD_LALT,
  MOD_RSHIFT,
  MOD_RCTRL,
  MOD_RALT,

  MOUSE_LEFT,
  MOUSE_RIGHT,
  MOUSE_MIDDLE,
  MOUSE_WHEEL_DOWN,
  MOUSE_WHEEL_UP,
  MOUSE_X1,
  MOUSE_X2,

  LENGTH
};
struct MouseData {
  float x, y;
  float xm, ym;
};
enum EError : uint8_t {
  INPUT_ERROR_INIT = 0,
  INPUT_ERROR_UPDATE,        // while processing events from SDL
  INPUT_ERROR_SET_MOUSE_POS, // while we were setting the mouse pos
};
using Error = ErrorBase<EError>;
Error init(Input &ctx);
Error deinit(Input &ctx);
Error update(Input &ctx);

// checks to see if the input is down
bool mapping_is_down(const Input &ctx, EMapping input);
// checks to see if the input was pressed
bool mapping_was_pressed(const Input &ctx, EMapping input);

Error set_mouse_pos(Input &ctx, uint32_t x_pos, uint32_t y_pos);
Error center_mouse_pos(Input &ctx);
const MouseData &get_mouse_data(Input &ctx);

void set_input_key(EMapping input, char key);

} // namespace input
struct Input {
  input::MouseData m_mouse_data{};
  std::bitset<input::EMapping::LENGTH> m_input_state{0};
  std::bitset<input::EMapping::LENGTH> m_last_input_state{0};
  std::reference_wrapper<Window> m_window;
};
using InputError = input::Error;
