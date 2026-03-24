#pragma once
#include "Subsystem.h"

#define ERROR_ENTRIES(E)                                                                           \
  E(INIT, "initializing the window")                                                               \
  E(SHOW, "showing the window")                                                                    \
  E(HIDE, "hiding the window")                                                                     \
  E(MOVE, "moving the window")                                                                     \
  E(RESIZE, "resizing the window")
DEFINE_DERIVED_ERROR_TYPES(Window, Subsystem, ERROR_ENTRIES);
#undef ERROR_ENTRIES

#define WINDOW_START_WIDTH 960
#define WINDOW_START_HEIGHT 540

class Window : public Subsystem<WindowError> {
  APPLICATION_PARENT(Window)

  uint32_t m_y{}, m_x{};
  DEFINE_PROPERTY(uint32_t, m_width, getWidth, setWidth, 0);
  DEFINE_PROPERTY(uint32_t, m_height, getHeight, setHeight, 0);

  bool m_resized{}, m_focused{};
  DEFINE_PROPERTY(bool, m_ticking, getTicking, setTicking, {});

  std::shared_ptr<SDL_Window> m_handle;
  DEFINE_REF_PROPERTY(std::string, m_name, getName, setName, {});

public:
  /// Show the window
  Error show();
  /// Close the window
  Error close();
  /// Demand focus on the window
  Error raise();
  /// Move the window
  Error move(uint32_t t_x, uint32_t t_y);
  /// Resize the window
  Error resize(uint32_t t_width, uint32_t t_height);
  // Raw pointer is only for interfacing with SDL
  SDL_Window *getRawHandle();

private:
  ApplicationError event(Event &t_evt);
};
