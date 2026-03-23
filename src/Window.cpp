#include "Window.h"
#include <print>
using enum EWindowError;
using Error = WindowError;

// NOLINTBEGIN(cppcoreguidelines-narrowing-conversions)
Error Window::onInit() {
  auto primaryDisplayScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

  auto *handle = SDL_CreateWindow(
      m_name.c_str(), static_cast<int>(static_cast<float>(m_width) * primaryDisplayScale),
      static_cast<int>(static_cast<float>(m_height) * primaryDisplayScale),
      SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY);
  if (handle == nullptr) {
    return {INIT, SDL_GetError()};
  }
  m_handle = std::shared_ptr<SDL_Window>(handle, SDL_DestroyWindow);
  if (auto error = move(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED); error) {
    auto [type, msg] = *error;
    std::println(stderr, "Warning from {}: {}", error.context(), msg);
  }
  return {};
}
// NOLINTEND

Error Window::onDestroy() {
  (void)this;
  return {};
}

Error Window::onUpdate() {
  // i don't know what we would need to update here rn
  // may remove method
  (void)this;
  return {};
}

// ----------

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

Error Window::move(uint32_t t_x, uint32_t t_y) {
  m_y = t_x;
  m_x = t_y;
  if (!SDL_SetWindowPosition(getRawHandle(), static_cast<int>(t_x), static_cast<int>(t_y))) {
    return {MOVE, SDL_GetError()};
  }
  return {};
}

Error Window::resize(uint32_t t_width, uint32_t t_height) {
  PASS_ERROR(setWidth(t_width))
  PASS_ERROR(setHeight(t_height))
  if (!SDL_SetWindowSize(getRawHandle(), static_cast<int>(m_width), static_cast<int>(m_height))) {
    return {RESIZE, SDL_GetError()};
  }
  return {};
}

Error Window::event(const SDL_WindowEvent &t_evt) {
  switch (t_evt.type) {
  case SDL_EVENT_WINDOW_FOCUS_LOST:
    m_focused = false;
    break;
  case SDL_EVENT_WINDOW_FOCUS_GAINED:
    m_focused = true;
    break;
  case SDL_EVENT_WINDOW_RESIZED:
    m_width = t_evt.data1;
    m_height = t_evt.data2;
    // glViewport(0, 0, ctx.width, ctx.height);
    m_resized = true;
    break;
  default:
    break;
  }
  return {};
}

SDL_Window *Window::getRawHandle() { return m_handle.get(); }
