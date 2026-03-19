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

constexpr int WINDOW_START_WIDTH = 960;
constexpr int WINDOW_START_HEIGHT = 540;

class Window : public Subsystem<WindowError> {
  friend class Subsystem;

public:
  using Error = WindowError;

  using Mama = Subsystem<Error>;

  /// Show the window
  Error show();
  /// Close the window
  Error close();
  /// Demand focus on the window
  Error raise();
  /// Move the window
  Error move(uint32_t x_pos, uint32_t y_pos);
  /// Resize the window
  Error resize(uint32_t width, uint32_t height);
  /// Process incoming `SDL_WindowEvent`s
  Error event(const SDL_WindowEvent &event);
  // Raw pointer is only for interfacing with SDL
  SDL_Window *getRawHandle();

private:
  Error onInit();
  Error onDestroy();
  Error onUpdate();

  uint32_t m_x_pos{}, m_y_pos{};
  DEFINE_PROPERTY(uint32_t, m_width, getWidth, setWidth, 0);
  DEFINE_PROPERTY(uint32_t, m_height, getHeight, setHeight, 0);

  bool m_resized{}, m_focused{};
  DEFINE_PROPERTY(bool, m_ticking, getTicking, setTicking, {});

  std::shared_ptr<SDL_Window> m_handle;
  DEFINE_REF_PROPERTY(std::string, m_name, getName, setName, {});
};
