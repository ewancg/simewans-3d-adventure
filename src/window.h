#pragma once
#include "graphics.h"

#define WINDOW_START_WIDTH 960
#define WINDOW_START_HEIGHT 540

namespace window {
enum class EError : uint8_t {
  INIT = 0,
  SHOW,
  HIDE,
};

ERROR_CONTEXT_TYPE({
case INIT:
  return "initializing the window";
case SHOW:
  return "showing the window";
case HIDE:
  return "hiding the window";
})

/// Given a pre-populated m_graphics; create a window, have the GPU "claim" the window, create the swapchain connecting
/// the two, and create the swapchain's texture
Error init(Window &ctx);

/// Show the window
Error show(Window &ctx);

/// Re-render the window
Error update(Window &ctx);

/// Hide the window
Error hide(Window &ctx);

/// Handle `SDL_WindowEvent`s
Error handle_event(Window &ctx, SDL_WindowEvent event);

// Raw pointer is only for interfacing with SDL
SDL_Window *get_handle(Window &ctx);
} // namespace window

struct Window {
  uint32_t m_width{}, m_height{};
  bool m_resized{}, m_focused{}, m_ticking{};
  std::shared_ptr<SDL_Window> m_handle;
  std::reference_wrapper<Graphics> m_graphics;
  std::string m_name;
};
using WindowError = window::Error;
