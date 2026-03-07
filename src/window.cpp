#include "window.h"

namespace window {
using enum EError;
Error init(Window &ctx) {
  auto graphics = ctx.m_graphics.get();
  if (!graphics.m_handle) {
    return {INIT, "Called window::init without an existing Graphics"};
  }
  auto window = std::shared_ptr<SDL_Window>(SDL_CreateWindow(ctx.m_name.c_str(), static_cast<int>(ctx.m_width),
                                                             static_cast<int>(ctx.m_height), SDL_WINDOW_RESIZABLE),
                                            SDL_DestroyWindow);
  if (!window) {
    return {INIT, SDL_GetError()};
  }

  ctx.m_handle = window;
  ctx.m_width = WINDOW_START_WIDTH;
  ctx.m_height = WINDOW_START_HEIGHT;

  return {};
};

Error show(Window &ctx) {
  if (!SDL_ShowWindow(ctx.m_handle.get())) {
    return {SHOW, SDL_GetError()};
  }
  ctx.m_ticking = true;
  return {};
}

Error raise(Window &ctx) {
  if (!SDL_RaiseWindow(ctx.m_handle.get())) {
    return {SHOW, SDL_GetError()};
  }
  return {};
}

Error hide(Window &ctx) {
  if (!SDL_HideWindow(ctx.m_handle.get())) {
    return {HIDE, SDL_GetError()};
  }
  ctx.m_ticking = false;
  return {};
}

Error update(Window &ctx) {
  (void)ctx;
  // TODO: render
  return {};
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
  return {};
}
} // namespace window
