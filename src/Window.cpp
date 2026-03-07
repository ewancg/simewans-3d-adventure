#include "Window.h"
#include <print>
using enum EWindowError;
using Error = WindowError;

// NOLINTBEGIN(cppcoreguidelines-narrowing-conversions)
Error Window::onInit() {
  auto main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

  auto *handle =
      SDL_CreateWindow(m_name.c_str(), static_cast<int>(static_cast<float>(m_width) * main_scale),
                       static_cast<int>(static_cast<float>(m_height) * main_scale),
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
  // i don't know what we would need to update here rn
  // may remove method
  return {};
}

Error Window::move(uint32_t x_pos, uint32_t y_pos) {
  m_x_pos = x_pos;
  m_y_pos = y_pos;
  if (!SDL_SetWindowPosition(getRawHandle(), static_cast<int>(x_pos), static_cast<int>(y_pos))) {
    return {MOVE, SDL_GetError()};
  }
  return {};
}

Error Window::resize(uint32_t width, uint32_t height) {
  setWidth(width);
  setHeight(height);
  if (!SDL_SetWindowSize(getRawHandle(), static_cast<int>(m_width), static_cast<int>(m_height))) {
    return {RESIZE, SDL_GetError()};
  }
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
