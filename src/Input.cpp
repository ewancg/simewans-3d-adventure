#include "Input.h"
#include "Application.h"
using enum EInputError;
using Error = InputError;

Error Input::onInit() {
  if (!SDL_InitSubSystem(SDL_INIT_EVENTS | SDL_INIT_GAMEPAD)) {
    return {INIT, SDL_GetError()};
  }
  m_app.subscribeToEvents(this, &Input::keyboardEvent, SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP);
  m_app.subscribeToEvents(this, &Input::mouseMotionEvent, SDL_EVENT_MOUSE_MOTION,
                          SDL_EVENT_MOUSE_MOTION);
  m_app.subscribeToEvents(this, &Input::mouseButtonEvent, SDL_EVENT_MOUSE_BUTTON_DOWN,
                          SDL_EVENT_MOUSE_WHEEL);
  return {};
}

Error Input::onDestroy() {
  SDL_QuitSubSystem(SDL_INIT_EVENTS | SDL_INIT_GAMEPAD);
  return {};
}

ApplicationError Input::keyboardEvent(Event &t_evt) {
  auto evt = t_evt.second.key;
#define KEYS(NAME) INPUT_CASE(KEY, SDL_SCANCODE, m_inputState, getKeyState(evt), NAME)
#define MODS(NAME) INPUT_CASE(MOD, SDL_SCANCODE, m_inputState, getKeyState(evt), NAME)
  switch (evt.scancode) {
    KB_INPUT_KEY_CASES(KEYS)
    KB_INPUT_MODIFIER_CASES(MODS)
  default:
    return {};
  }
  t_evt.consume();
  return {};
}

ApplicationError Input::mouseButtonEvent(Event &t_evt) {
  auto evt = t_evt.second;
#define MOUSE_BUTTONS(NAME) INPUT_CASE(MOUSE, SDL_BUTTON, m_inputState, evt.button.down, NAME)
  switch (evt.button.type) {
    MOUSE_INPUT_BUTTON_CASES(MOUSE_BUTTONS)
  case SDL_EVENT_MOUSE_WHEEL:
    if (evt.wheel.y < 0) {
      m_inputState[MOUSE_WHEEL_DOWN] = true;
    }
    if (evt.wheel.y > 0) {
      m_inputState[MOUSE_WHEEL_UP] = true;
    }
  default:
    return {};
  }
  t_evt.consume();
  return {};
}

ApplicationError Input::mouseMotionEvent(Event &t_evt) {
  const auto display = SDL_GetDisplayForWindow(m_window.getRawHandle());
  if (display == 0) {
    return {EVENT, SDL_GetError()};
  }
  const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(display);
  if (mode == nullptr) {
    return {EVENT, SDL_GetError()};
  }
  auto &evt = t_evt.second;
  auto &motion = evt.motion;
  m_mouseData.x = motion.x / static_cast<float>(mode->w);
  m_mouseData.y = motion.y / static_cast<float>(mode->h);
  m_mouseData.xm = motion.xrel / static_cast<float>(mode->w);
  m_mouseData.ym = motion.yrel / static_cast<float>(mode->h);

  t_evt.consume();
  return {};
}

Error Input::onUpdate() {
  m_lastInputState = m_inputState;
  return {};
}

// ----------

bool Input::getKeyState(const SDL_KeyboardEvent &t_evt) { return t_evt.down || !t_evt.repeat; };

constexpr const char *mousePosErrMsg =
    "attempted to set the mouse position on an Input without a valid window";
Error Input::setMousePos(uint32_t t_x, uint32_t t_y) {
  auto *handle = m_window.getRawHandle();
  if (handle == nullptr) {
    return {SET_MOUSE_POS, mousePosErrMsg};
  }
  SDL_WarpMouseInWindow(handle, static_cast<float>(t_x), static_cast<float>(t_y));
  return {};
}
Error Input::centerMousePos() {
  auto &window = m_window;
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

  SDL_WarpMouseInWindow(handle, MIDDLE_F(width), MIDDLE_F(height));
  return {};
}
bool Input::mappingIsPressed(EInputMapping t_in) { return m_inputState[t_in]; }
bool Input::mappingNewlyPressed(EInputMapping t_in) {
  return m_inputState[t_in] && !m_lastInputState[t_in];
}
