#include "graphics.h"
#include "window.h"

#ifndef GRAPHICS_DEBUGGING
#define GRAPHICS_DEBUGGING false
#endif

namespace graphics {
using enum EError;
Error init(Graphics &ctx) {
  if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
    return {SUBSYSTEM_INIT, SDL_GetError()};
  }
  auto *device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
                                     GRAPHICS_DEBUGGING, nullptr);
  if (device == nullptr) {
    return {GPU_INIT, SDL_GetError()};
  }

  auto *cmd_buf = SDL_AcquireGPUCommandBuffer(device);
  if (cmd_buf == nullptr) {
    return {GPU_INIT_CMDBUF, SDL_GetError()};
  }

  ctx.m_handle = std::shared_ptr<SDL_GPUDevice>(device, SDL_DestroyGPUDevice);
  ctx.m_main_command_buffer = std::shared_ptr<SDL_GPUCommandBuffer>(cmd_buf, [](auto) {
    // SDL explicitly says not to free this
  });

  return {};
};

Error attach_window(Graphics &ctx, Window &window) {
  auto *gpu = ctx.m_handle.get();
  auto *win = window.m_handle.get();

  if (!SDL_ClaimWindowForGPUDevice(gpu, win)) {
    return {GPU_WINDOW_CLAIM, SDL_GetError()};
  }
  if (!SDL_SetGPUSwapchainParameters(gpu, win, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_IMMEDIATE)) {
    return {SET_SWAPCHAIN_PARAMS, SDL_GetError()};
  }
  return {};
}

Error get_window_swapchain_texture(Graphics &ctx, Window &window, SDL_GPUTexture *output) {
  if (!ctx.m_main_command_buffer) {
    return {GET_WINDOW_SWAPCHAIN_TEXTURE, "graphics::get_swapchain_texture called without a rendering command buffer"};
  }
  auto *cmd_buf = ctx.m_main_command_buffer.get();
  if (!window.m_handle) {
    return {GET_WINDOW_SWAPCHAIN_TEXTURE, "graphics::get_swapchain_texture called without a window"};
  }
  auto *win = window.m_handle.get();

  if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd_buf, win, &output, &window.m_width, &window.m_height)) {
    return {GET_WINDOW_SWAPCHAIN_TEXTURE, SDL_GetError()};
  }

  return {};
}

} // namespace graphics
