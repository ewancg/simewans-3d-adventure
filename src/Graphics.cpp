#include "Graphics.h"
using enum EGraphicsError;
using Error = GraphicsError;

#ifndef GRAPHICS_DEBUGGING
#define GRAPHICS_DEBUGGING false
#endif

Error Graphics::onInit() {
  if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
    return {INIT, SDL_GetError()};
  }

  auto *device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL |
                                         SDL_GPU_SHADERFORMAT_METALLIB,
                                     GRAPHICS_DEBUGGING, nullptr);
  if (device == nullptr) {
    return {GPU_INIT, SDL_GetError()};
  }

  m_device = std::shared_ptr<SDL_GPUDevice>(device, SDL_DestroyGPUDevice);
  return {};
}

Error Graphics::onDestroy() {
  SDL_DestroyGPUDevice(m_device.get());
  // Explicitly do not free command buffers
  return {};
}

Error Graphics::beginFrame(SDL_GPUTexture *tex) {
  for (auto &command_buffer : m_command_buffers) {
    command_buffer = SDL_AcquireGPUCommandBuffer(m_device.get());
    if (command_buffer == nullptr) {
      return {GPU_INIT_CMDBUF, SDL_GetError()};
    }
  }

  auto *cmd_buf = m_command_buffers[RENDER];
  if (cmd_buf == nullptr) {
    return {FRAME_BEGIN, "command buffer somehow invalidated"};
  }

  // Setup and start a render pass
  SDL_GPUColorTargetInfo target_info{.texture = tex,
                                     .mip_level = 0,
                                     .layer_or_depth_plane = 0,
                                     .clear_color = {.r = 0.6F, .g = 0.2F, .b = 0.2F},
                                     .load_op = SDL_GPU_LOADOP_CLEAR,
                                     .store_op = SDL_GPU_STOREOP_STORE,
                                     .cycle = false};
  m_render_pass = SDL_BeginGPURenderPass(cmd_buf, &target_info, 1, nullptr);
  return {};
}
double Graphics::endFrame(uint64_t frame_interval) {
  SDL_EndGPURenderPass(m_render_pass);

  for (const auto &item : m_command_buffers) {
    SDL_SubmitGPUCommandBuffer(item);
  }

  uint64_t now = SDL_GetTicksNS();
  uint64_t frametime = now - m_last_time;
  // printf("%u\n", frametime.count());

  SDL_DelayPrecise(frame_interval - frametime);

  m_last_time = now;
  m_render_pass = nullptr;
  return static_cast<double>(frame_interval) / TICK_DIVISOR;
}

Error Graphics::attachWindow(Window &window) {
  auto *gpu = m_device.get();
  auto *win = window.getRawHandle();

  if (!SDL_ClaimWindowForGPUDevice(gpu, win)) {
    return {GPU_WINDOW_CLAIM, SDL_GetError()};
  }
  if (!SDL_SetGPUSwapchainParameters(gpu, win, SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
                                     SDL_GPU_PRESENTMODE_IMMEDIATE)) {
    return {SET_SWAPCHAIN_PARAMS, SDL_GetError()};
  }
  return {};
}

#define BAD_CALL(MSG)                                                                              \
  {GET_WINDOW_SWAPCHAIN_TEXTURE, "graphics::get_swapchain_texture called without a " MSG}
Error Graphics::getWindowSwapchainTexture(Window &window, SDL_GPUTexture *output) {
  auto *cmd_buf = m_command_buffers[ECommandBufferRole::RENDER];
  if (cmd_buf == nullptr) {
    return BAD_CALL("rendering command buffer");
  }
  if (output == nullptr) {
    return BAD_CALL("destination texture");
  }
  auto *win = window.getRawHandle();
  if (win == nullptr) {
    return BAD_CALL("subject window");
  }
  if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd_buf, win, &output, nullptr, nullptr)) {
    return {GET_WINDOW_SWAPCHAIN_TEXTURE, SDL_GetError()};
  }
  return {};
}
#undef BAD_CALL
