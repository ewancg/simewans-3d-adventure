#include "window.h"
using enum EWindowError;
using Error = WindowError;

Error Window::onInit() {
  m_handle = std::shared_ptr<SDL_Window>(SDL_CreateWindow(m_name.c_str(), static_cast<int>(m_width),
                                                          static_cast<int>(m_height),
                                                          SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN),
                                         SDL_DestroyWindow);
  if (!m_handle) {
    return {INIT, SDL_GetError()};
  }
  return {};
}

Error Window::show() {
  if (!SDL_ShowWindow(getRawHandle())) {
    return {SHOW, SDL_GetError()};
  }
  m_ticking = true;
  return {};
}

Error Window::close() {
  if (!SDL_HideWindow(getRawHandle())) {
    return {HIDE, SDL_GetError()};
  }
  m_ticking = false;
  return {};
}

Error Window::raise() {
  if (!SDL_RaiseWindow(getRawHandle())) {
    return {SHOW, SDL_GetError()};
  }
  return {};
}

Error Window::update() {
  // TODO: render
  return {};
}

Error Window::event(const SDL_WindowEvent &event) {
  switch (event.type) {
  case SDL_EVENT_WINDOW_FOCUS_LOST:
    m_focused = false;
    break;
  case SDL_EVENT_WINDOW_FOCUS_GAINED:
    m_focused = true;
    break;
  case SDL_EVENT_WINDOW_RESIZED:
    m_width = event.data1;
    m_height = event.data2;
    // glViewport(0, 0, ctx.width, ctx.height);
    m_resized = true;
    break;
  default:
    break;
  }
  return {};
}

SDL_Window *Window::getRawHandle() { return m_handle.get(); }
