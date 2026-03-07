#pragma once

#define WINDOW_START_WIDTH 960
#define WINDOW_START_HEIGHT 540

struct Window;
namespace window {
enum EError : uint8_t {
  // WINDOW_ERROR_INIT,
  WINDOW_ERROR_SHOW = 0,
  WINDOW_ERROR_HIDE,
};
using Error = ErrorBase<EError>;

void init(Window &ctx);
Error show(Window &ctx);
Error hide(Window &ctx);

Error handle_event(Window &ctx, SDL_WindowEvent event);

// Raw pointer is only for interfacing with SDL
SDL_Window *get_handle(Window &ctx);
} // namespace window

struct Window {
  uint32_t m_width{}, m_height{};
  bool m_resized{}, m_focused{}, m_ticking{};
  std::shared_ptr<SDL_Window> m_handle;
  std::string m_name;
};
using WindowError = window::Error;
