#include "Input.h"
#include <cstdint>
using enum EInputError;
using Error = InputError;

bool Input::getKeyState(const SDL_KeyboardEvent &event) { return event.down || !event.repeat; };

void Input::handleKeyboardEvent(const SDL_KeyboardEvent &event) {
#define KEYS(name) INPUT_CASE(KEY, SDL_SCANCODE, m_input_state, getKeyState(event), name)
#define MODS(name) INPUT_CASE(MOD, SDL_SCANCODE, m_input_state, getKeyState(event), name)
  switch (event.scancode) {
    KB_INPUT_KEY_CASES(KEYS)
    KB_INPUT_MODIFIER_CASES(MODS)
  default:
    break;
  }
}
void Input::handleMouseButtonEvent(const SDL_MouseButtonEvent &event) {
#define MOUSE_BUTTONS(name) INPUT_CASE(MOUSE, SDL_BUTTON, m_input_state, event.down, name)
  switch (event.button) {
    MOUSE_INPUT_BUTTON_CASES(MOUSE_BUTTONS)
  default:
    break;
  }
}

Error Input::onInit() {
  if (!SDL_InitSubSystem(SDL_INIT_EVENTS | SDL_INIT_GAMEPAD)) {
    return {INIT, SDL_GetError()};
  }
  return {};
}

constexpr const char *MOUSE_POS_ERR_MSG =
    "attempted to set the mouse position on an Input without a valid window";
Error Input::setMousePos(uint32_t x_pos, uint32_t y_pos) {
  auto *handle = m_window.get().getRawHandle();
  if (handle == nullptr) {
    return {SET_MOUSE_POS, MOUSE_POS_ERR_MSG};
  }
  SDL_WarpMouseInWindow(handle, static_cast<float>(x_pos), static_cast<float>(y_pos));
  return {};
}
Error Input::centerMousePos() {
  auto window = m_window.get();
  auto *handle = window.getRawHandle();
  if (handle == nullptr) {
    return {SET_MOUSE_POS, MOUSE_POS_ERR_MSG};
  }
  uint32_t width{}, height{};
  if (auto error = window.getWidth(width); error) {
    return {SET_MOUSE_POS, error.string()};
  }
  if (auto error = window.getHeight(width); error) {
    return {SET_MOUSE_POS, error.string()};
  }

  SDL_WarpMouseInWindow(handle, static_cast<float>(width) / 2.F, static_cast<float>(height) / 2.F);
  return {};
}
bool Input::mappingIsPressed(EInputMapping input) { return m_input_state[input]; }
bool Input::mappingNewlyPressed(EInputMapping input) {
  return m_input_state[input] && !m_last_input_state[input];
}

Error Input::handleMouseEvent(const SDL_Event &event) {
  const auto display = SDL_GetDisplayForWindow(m_window.get().getRawHandle());
  if (display == 0) {
    return {EVENT, SDL_GetError()};
  }
  const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(display);
  if (mode == nullptr) {
    return {EVENT, SDL_GetError()};
  }

  const auto &evt = event.motion;
  m_mouse_data.x = static_cast<float>(evt.x) / static_cast<float>(mode->w);
  m_mouse_data.y = static_cast<float>(evt.y) / static_cast<float>(mode->h);
  m_mouse_data.xm = static_cast<float>(evt.xrel) / static_cast<float>(mode->w);
  m_mouse_data.ym = static_cast<float>(evt.yrel) / static_cast<float>(mode->h);

  if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
    handleMouseButtonEvent(event.button);
  }
  if (event.type == SDL_EVENT_MOUSE_WHEEL) {
    if (event.wheel.y < 0) {
      m_input_state[MOUSE_WHEEL_DOWN] = true;
    }
    if (event.wheel.y > 0) {
      m_input_state[MOUSE_WHEEL_UP] = true;
    }
  }
  return {};
}

Error Input::onUpdate() {
  static const auto passive_error = [](auto err) {
    auto [type, str] = *err;
    auto type_str = err.context();
    std::println(stderr, "Input: error while {}: {}", type_str, str);
  };
  auto window = m_window.get();
  m_last_input_state = m_input_state;

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      window.setTicking(false);
    } else if (event.type >= SDL_EVENT_WINDOW_SHOWN && event.type <= SDL_EVENT_KEY_DOWN) {
      window.event(event.window).mapError(passive_error);
    } else if (event.type >= SDL_EVENT_KEY_DOWN && event.type <= SDL_EVENT_MOUSE_MOTION) {
      handleKeyboardEvent(event.key);
    } else if (event.type >= SDL_EVENT_MOUSE_MOTION &&
               event.type <= SDL_EVENT_JOYSTICK_AXIS_MOTION) {
      handleMouseEvent(event).mapError(passive_error);
    }
  }
  return {};
} // namespace input
