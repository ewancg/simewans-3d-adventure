#include "graphics.h"

namespace window {
void init(Window &ctx) {
  ctx.m_width = WINDOW_START_WIDTH;
  ctx.m_height = WINDOW_START_HEIGHT;
  ctx.m_handle = std::shared_ptr<SDL_Window>(
      SDL_CreateWindow(ctx.m_name.c_str(), ctx.m_width, ctx.m_height, SDL_WINDOW_RESIZABLE), SDL_DestroyWindow);
};

WindowError show(Window &ctx) {
  if (!SDL_ShowWindow(ctx.m_handle.get())) {
    return std::make_tuple(WINDOW_ERROR_SHOW, SDL_GetError());
  }
  ctx.m_ticking = true;
  return std::nullopt;
}

WindowError hide(Window &ctx) {
  if (!SDL_HideWindow(ctx.m_handle.get())) {
    return std::make_tuple(WINDOW_ERROR_HIDE, SDL_GetError());
  }
  ctx.m_ticking = false;
  return std::nullopt;
}

SDL_Window *get_handle(Window &ctx) { return ctx.m_handle.get(); }

WindowError handle_event(Window &ctx, SDL_WindowEvent event) {
  switch (event.type) {
  case SDL_EVENT_WINDOW_FOCUS_LOST:
    ctx.m_focused = false;
    break;
  case SDL_EVENT_WINDOW_FOCUS_GAINED:
    ctx.m_focused = true;
    break;
  case SDL_EVENT_WINDOW_RESIZED:
    ctx.m_width = event.data1;
    ctx.m_height = event.data2;
    // glViewport(0, 0, ctx.m_width, ctx.m_height);
    ctx.m_resized = true;
    break;
  default:
    break;
  }
  return std::nullopt;
}
} // namespace window
