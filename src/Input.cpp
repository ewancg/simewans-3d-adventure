#include "Input.h"
using enum EInputError;
using Error = InputError;

Error Input::onInit() {
  if (!SDL_InitSubSystem(SDL_INIT_EVENTS | SDL_INIT_GAMEPAD)) {
    return {INIT, SDL_GetError()};
  }
  return {};
}

Error Input::onDestroy() {
  SDL_QuitSubSystem(SDL_INIT_EVENTS | SDL_INIT_GAMEPAD);
  return {};
}

Error Input::onUpdate() {
  auto window = m_window.get();
  m_lastInputState = m_inputState;

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      window.setTicking(false).mapError(logPassiveError);
    } else if (event.type >= SDL_EVENT_WINDOW_SHOWN && event.type <= SDL_EVENT_KEY_DOWN) {
      window.event(event.window).mapError(logPassiveError);
    } else if (event.type >= SDL_EVENT_KEY_DOWN && event.type <= SDL_EVENT_MOUSE_MOTION) {
      handleKeyboardEvent(event.key);
    } else if (event.type >= SDL_EVENT_MOUSE_MOTION &&
               event.type <= SDL_EVENT_JOYSTICK_AXIS_MOTION) {
      handleMouseEvent(event).mapError(logPassiveError);
    }
  }
  return {};
}

// ----------

bool Input::getKeyState(const SDL_KeyboardEvent &t_evt) { return t_evt.down || !t_evt.repeat; };

void Input::handleKeyboardEvent(const SDL_KeyboardEvent &t_evt) {
#define KEYS(NAME) INPUT_CASE(KEY, SDL_SCANCODE, m_inputState, getKeyState(t_evt), NAME)
#define MODS(NAME) INPUT_CASE(MOD, SDL_SCANCODE, m_inputState, getKeyState(t_evt), NAME)
  switch (t_evt.scancode) {
    KB_INPUT_KEY_CASES(KEYS)
    KB_INPUT_MODIFIER_CASES(MODS)
  default:
    break;
  }
}
void Input::handleMouseButtonEvent(const SDL_MouseButtonEvent &t_evt) {
#define MOUSE_BUTTONS(NAME) INPUT_CASE(MOUSE, SDL_BUTTON, m_inputState, t_evt.down, NAME)
  switch (t_evt.button) {
    MOUSE_INPUT_BUTTON_CASES(MOUSE_BUTTONS)
  default:
    break;
  }
}
constexpr const char *mousePosErrMsg =
    "attempted to set the mouse position on an Input without a valid window";
Error Input::setMousePos(uint32_t t_x, uint32_t t_y) {
  auto *handle = m_window.get().getRawHandle();
  if (handle == nullptr) {
    return {SET_MOUSE_POS, mousePosErrMsg};
  }
  SDL_WarpMouseInWindow(handle, static_cast<float>(t_x), static_cast<float>(t_y));
  return {};
}
Error Input::centerMousePos() {
  auto window = m_window.get();
  auto *handle = window.getRawHandle();
  if (handle == nullptr) {
    return {SET_MOUSE_POS, mousePosErrMsg};
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
bool Input::mappingIsPressed(EInputMapping t_in) { return m_inputState[t_in]; }
bool Input::mappingNewlyPressed(EInputMapping t_in) {
  return m_inputState[t_in] && !m_lastInputState[t_in];
}

Error Input::handleMouseEvent(const SDL_Event &t_evt) {
  const auto display = SDL_GetDisplayForWindow(m_window.get().getRawHandle());
  if (display == 0) {
    return {EVENT, SDL_GetError()};
  }
  const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(display);
  if (mode == nullptr) {
    return {EVENT, SDL_GetError()};
  }

  const auto &evt = t_evt.motion;
  m_mouse_data.x = static_cast<float>(evt.x) / static_cast<float>(mode->w);
  m_mouse_data.y = static_cast<float>(evt.y) / static_cast<float>(mode->h);
  m_mouse_data.xm = static_cast<float>(evt.xrel) / static_cast<float>(mode->w);
  m_mouse_data.ym = static_cast<float>(evt.yrel) / static_cast<float>(mode->h);

  if (t_evt.type == SDL_EVENT_MOUSE_BUTTON_DOWN || t_evt.type == SDL_EVENT_MOUSE_BUTTON_UP) {
    handleMouseButtonEvent(t_evt.button);
  }
  if (t_evt.type == SDL_EVENT_MOUSE_WHEEL) {
    if (t_evt.wheel.y < 0) {
      m_inputState[MOUSE_WHEEL_DOWN] = true;
    }
    if (t_evt.wheel.y > 0) {
      m_inputState[MOUSE_WHEEL_UP] = true;
    }
  }
  return {};
}
