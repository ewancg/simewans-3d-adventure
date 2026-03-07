#pragma once
#include "window.h"
#include <bitset>

namespace input {
enum class EError : uint8_t {
  INIT = 0,
  SUBSYSTEM_INIT,
  UPDATE,
  SET_MOUSE_POS,
};
ERROR_CONTEXT_TYPE({
case SUBSYSTEM_INIT:
  return "initializing input subsystem";
case UPDATE:
  return "processing input events";
case SET_MOUSE_POS:
  return "setting the mouse cursor position";
})
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

Error init(Input &ctx);
Error deinit(Input &ctx);
Error update(Input &ctx);

/// Checks if the input is currently down
bool mapping_is_pressed(const Input &ctx, EMapping input);
/// Checks if the input is currently down, and was not last frame
bool mapping_newly_pressed(const Input &ctx, EMapping input);
/// Sets the mouse cursor to an absolute position
Error set_mouse_pos(Input &ctx, uint32_t x_pos, uint32_t y_pos);
/// Centers the cursor within the screen
Error center_mouse_pos(Input &ctx);
} // namespace input
struct Input {
  input::MouseData m_mouse_data{};
  std::bitset<input::EMapping::LENGTH> m_input_state{0};
  std::bitset<input::EMapping::LENGTH> m_last_input_state{0};
  std::reference_wrapper<Window> m_window;
};
using InputError = input::Error;
