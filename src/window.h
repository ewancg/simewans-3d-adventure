#pragma once

#define ERRORS(E)                                                                                  \
  E(INIT, "initializing the window")                                                               \
  E(SHOW, "showing the window")                                                                    \
  E(HIDE, "hiding the window")
DEFINE_ERROR_TYPES(Window, ERRORS);
#undef ERRORS

constexpr int WINDOW_START_WIDTH = 960;
constexpr int WINDOW_START_HEIGHT = 540;

class Window : public Subsystem<WindowError> {
  using Error = WindowError;

  DEFINE_PROPERTY(uint32_t, m_width, width, setWidth);
  DEFINE_PROPERTY(uint32_t, m_height, height, setHeight);

  bool m_resized{}, m_focused{};
  DEFINE_PROPERTY(bool, m_ticking, ticking, setTicking);

  std::shared_ptr<SDL_Window> m_handle;
  DEFINE_REF_PROPERTY(std::string, m_name, name, setName);

public:
  Error onInit();
  Error onDestroy();
  /// Show the window
  Error show();
  /// Close the window
  Error close();
  /// Demand focus on the window
  Error raise();
  /// Tick the window
  Error update();
  /// Process incoming `SDL_WindowEvent`s
  Error event(const SDL_WindowEvent &event);
  // Raw pointer is only for interfacing with SDL
  SDL_Window *getRawHandle();
};
