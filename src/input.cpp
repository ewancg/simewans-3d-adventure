#include "input.h"

// clang-format off
#define INPUT_CASE(PREFIX, SDL_PREFIX, STATE, RHS, NAME)   \
  case SDL_PREFIX##_##NAME:                                \
    (STATE)[PREFIX##_##NAME] = (RHS);                      \
    break;

#define KB_INPUT_KEY_CASES(FN) \
  FN(A) FN(B) FN(C) FN(D) FN(E) FN(F) FN(G) FN(H) FN(I) FN(J) FN(K) FN(L) FN(M) \
  FN(N) FN(O) FN(P) FN(Q) FN(R) FN(S) FN(T) FN(U) FN(V) FN(W) FN(X) FN(Y) FN(Z) \
  FN(0) FN(1) FN(2) FN(3) FN(4) FN(5) FN(6) FN(7) FN(8) FN(9)                \
  FN(RETURN) FN(SPACE) FN(ESCAPE) FN(LEFT) FN(RIGHT) FN(UP) FN(DOWN)

#define KB_INPUT_MODIFIER_CASES(FN) \
  FN(LSHIFT) FN(LCTRL) FN(LALT) FN(RSHIFT) FN(RCTRL) FN(RALT)

#define MOUSE_INPUT_BUTTON_CASES(FN) \
  FN(LEFT) FN(MIDDLE) FN(RIGHT) FN(X1) FN(X2)
// clang-format on

bool get_key_state(const SDL_KeyboardEvent &event) { return event.down || !event.repeat; };

namespace input {
using enum EError;
Error init(Input &ctx) {
  auto &window = ctx.m_window.get();
  if (!window.m_handle) {
    return {INIT, "Called input::init without an existing Window"};
  }
  if (!SDL_InitSubSystem(SDL_INIT_EVENTS)) {
    return {SUBSYSTEM_INIT, SDL_GetError()};
  }
  return {};
}

Error set_mouse_pos(Input &ctx, uint32_t x_pos, uint32_t y_pos) {
  auto &window = ctx.m_window.get();
  if (!window.m_handle) {
    return {SET_MOUSE_POS, "Attempted to set the mouse position on an Input without a valid window"};
  }
  SDL_WarpMouseInWindow(window.m_handle.get(), static_cast<float>(x_pos), static_cast<float>(y_pos));
  return {};
}
Error center_mouse_pos(Input &ctx) {
  auto &window = ctx.m_window.get();
  if (!window.m_handle) {
    return {SET_MOUSE_POS, "Attempted to set the mouse position on an Input without a valid window"};
  }

  SDL_WarpMouseInWindow(window.m_handle.get(), static_cast<float>(window.m_width) / 2.F,
                        static_cast<float>(window.m_height) / 2.F);
  return {};
}
bool mapping_is_pressed(const Input &ctx, EMapping input) { return ctx.m_input_state[input]; }
bool mapping_newly_pressed(const Input &ctx, EMapping input) {
  return ctx.m_input_state[input] && !ctx.m_last_input_state[input];
}

void handle_keyboard_event(Input &ctx, const SDL_KeyboardEvent &event) {
#define KEYS(name) INPUT_CASE(KEY, SDL_SCANCODE, ctx.m_input_state, get_key_state(event), name)
#define MODS(name) INPUT_CASE(MOD, SDL_SCANCODE, ctx.m_input_state, get_key_state(event), name)
  switch (event.scancode) {
    KB_INPUT_KEY_CASES(KEYS)
    KB_INPUT_MODIFIER_CASES(MODS)
  default:
    break;
  }
}
void handle_mouse_button_event(Input &ctx, const SDL_MouseButtonEvent &event) {
#define MOUSE_BUTTONS(name) INPUT_CASE(MOUSE, SDL_BUTTON, ctx.m_input_state, event.down, name)
  switch (event.button) {
    MOUSE_INPUT_BUTTON_CASES(MOUSE_BUTTONS)
  default:
    break;
  }
}

Error handle_mouse_event(Input &ctx, const SDL_Event &event) {
  const auto display = SDL_GetDisplayForWindow(window::get_handle(ctx.m_window));
  if (display == 0) {
    return {UPDATE, SDL_GetError()};
  }
  const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(display);
  if (mode == nullptr) {
    return {UPDATE, SDL_GetError()};
  }

  auto &mdata = ctx.m_mouse_data;
  const auto &evt = event.motion;

  mdata.x = static_cast<float>(evt.x) / static_cast<float>(mode->w);
  mdata.y = static_cast<float>(evt.y) / static_cast<float>(mode->h);
  mdata.xm = static_cast<float>(evt.xrel) / static_cast<float>(mode->w);
  mdata.ym = static_cast<float>(evt.yrel) / static_cast<float>(mode->h);

  if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
    input::handle_mouse_button_event(ctx, event.button);
  }
  if (event.type == SDL_EVENT_MOUSE_WHEEL) {
    if (event.wheel.y < 0) {
      ctx.m_input_state[MOUSE_WHEEL_DOWN] = true;
    }
    if (event.wheel.y > 0) {
      ctx.m_input_state[MOUSE_WHEEL_UP] = true;
    }
  }
  return {};
}

InputError update(Input &ctx) {
  auto window = std::unwrap_reference_t<Window>(ctx.m_window);
  ctx.m_last_input_state = ctx.m_input_state;

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      window.m_ticking = false;
    } else if (event.type >= SDL_EVENT_WINDOW_SHOWN && event.type <= SDL_EVENT_KEY_DOWN) {
      window::handle_event(ctx.m_window, event.window);
    } else if (event.type >= SDL_EVENT_KEY_DOWN && event.type <= SDL_EVENT_MOUSE_MOTION) {
      input::handle_keyboard_event(ctx, event.key);
    } else if (event.type >= SDL_EVENT_MOUSE_MOTION && event.type <= SDL_EVENT_JOYSTICK_AXIS_MOTION) {
      if (auto error = input::handle_mouse_event(ctx, event); error) {
        return error;
      }
    }
  }
  return {};
}
} // namespace input
