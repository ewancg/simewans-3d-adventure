#include "Graphics.h"
using enum EGraphicsError;
using Error = GraphicsError;

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

Error Graphics::onUpdate() { return {}; }

Error Graphics::beginFrame(SDL_GPUTexture *tex) {
  //  if (auto err = (Subsystem::ensureInitialized)(&*this, "beginFrame called prematurely"); err) {
  //    return err;
  //  }
  for (auto &item : m_commandBuffers) {
    item = SDL_AcquireGPUCommandBuffer(m_device.get());
    if (item == nullptr) {
      return {GPU_INIT_CMDBUF, SDL_GetError()};
    }
  }
  auto *commandBuf = m_commandBuffers[RENDER];

  // Setup and start a render pass
  SDL_GPUColorTargetInfo targetInfo{.texture = tex,
                                    .mip_level = 0,
                                    .layer_or_depth_plane = 0,
                                    .clear_color = {.r = 0.6F, .g = 0.2F, .b = 0.2F},
                                    .load_op = SDL_GPU_LOADOP_CLEAR,
                                    .store_op = SDL_GPU_STOREOP_STORE,
                                    .cycle = false};
  m_renderPass = SDL_BeginGPURenderPass(commandBuf, &targetInfo, 1, nullptr);
  return {};
}
Error Graphics::endFrame(double &t_frameTimeOut) {
  SDL_EndGPURenderPass(m_renderPass);

  for (const auto &item : m_commandBuffers) {
    if (auto err = SDL_SubmitGPUCommandBuffer(item); err) {
      return {FRAME_FINALIZE, SDL_GetError()};
    }
  }

  uint64_t now = SDL_GetTicksNS();
  uint64_t frameTime = now - m_lastFrameTime;

  SDL_DelayPrecise(static_cast<uint64_t>(m_frameIntervalNS) - frameTime);

  m_lastFrameTime = now;
  t_frameTimeOut = m_frameIntervalNS / NS_PER_SEC;
  m_renderPass = nullptr;
  return {};
}

Error Graphics::attachWindow(Window &t_windowIn) {
  auto *gpu = m_device.get();
  auto *win = t_windowIn.getRawHandle();

  if (!SDL_ClaimWindowForGPUDevice(gpu, win)) {
    return {GPU_WINDOW_CLAIM, SDL_GetError()};
  }
  if (!SDL_SetGPUSwapchainParameters(gpu, win, SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
                                     SDL_GPU_PRESENTMODE_IMMEDIATE)) {
    return {SET_SWAPCHAIN_PARAMS, SDL_GetError()};
  }
  return {};
}

// template <typename T>
// concept IsEError = std::is_base_of<uint8_t, T>();
// template <IsEError T>

template <typename T = Error> static constexpr T badCall(const std::string &t_msg) {
  return {GET_WINDOW_SWAPCHAIN_TEXTURE,
          "graphics::get_swapchain_texture called without a " + t_msg};
}
Error Graphics::getWindowSwapchainTexture(Window &t_windowIn, SDL_GPUTexture *t_textureOut) {
  auto *commandBuf = m_commandBuffers[ECommandBufferRole::RENDER];
  if (commandBuf == nullptr) {
    return badCall("rendering command buffer");
  }
  if (t_textureOut == nullptr) {
    return badCall("destination texture");
  }
  auto *window = t_windowIn.getRawHandle();
  if (window == nullptr) {
    return badCall("subject window");
  }
  if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuf, window, &t_textureOut, nullptr, nullptr)) {
    return {GET_WINDOW_SWAPCHAIN_TEXTURE, SDL_GetError()};
  }
  return {};
}
#undef BAD_CALL
