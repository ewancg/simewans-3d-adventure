#include "Graphics.h"

using enum EGraphicsError;
using Error = GraphicsError;

Error Graphics::onInit() {
  /// Device
  if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
    return {INIT, SDL_GetError()};
  }
  auto *device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL |
                                         SDL_GPU_SHADERFORMAT_METALLIB,
                                     GRAPHICS_DEBUGGING, nullptr);
  /* CREATE SDL GPU DEVICE */
  if (device == nullptr) {
    return {GPU_INIT, SDL_GetError()};
  }

  /// Buffers
  /* CREATE SDL GPU TRANSFER BUFFERS */
  SDL_GPUTransferBufferCreateInfo uploadBufCreateInfo = {.usage =
                                                             SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                                                         .size = DEFAULT_TRANSFER_BUFFER_SIZE,
                                                         .props = 0};
  m_uploadBuffer = SDL_CreateGPUTransferBuffer(device, &uploadBufCreateInfo);
  if (m_uploadBuffer == nullptr) {
    return {GPU_INIT, SDL_GetError()};
  }

  SDL_GPUTransferBufferCreateInfo downloadBufCreateInfo = {.usage =
                                                               SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD,
                                                           .size = DEFAULT_TRANSFER_BUFFER_SIZE,
                                                           .props = 0};
  m_downloadBuffer = SDL_CreateGPUTransferBuffer(device, &downloadBufCreateInfo);
  if (m_downloadBuffer == nullptr) {
    return {GPU_INIT, SDL_GetError()};
  }

  m_device = std::shared_ptr<SDL_GPUDevice>(device, SDL_DestroyGPUDevice);
  return {};
}

Error Graphics::onDestroy() {

  /* DESTROY SDL GPU TRANSFER BUFFERS */
  if (m_uploadBuffer != nullptr) {
    SDL_ReleaseGPUTransferBuffer(m_device.get(), m_uploadBuffer);
  }
  if (m_downloadBuffer != nullptr) {
    SDL_ReleaseGPUTransferBuffer(m_device.get(), m_downloadBuffer);
  }
  if (m_device) {
    SDL_DestroyGPUDevice(m_device.get());
  }
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
  return {};
}

Error Graphics::onUpdate() { return {}; }

// ----------

Error Graphics::beginFrame(Window &t_windowIn) {
  PASS_ERROR(ensureInitialized<GraphicsError>(*this, "beginFrame called"))
  for (auto *item : m_commandBuffers) {
    if (item = SDL_AcquireGPUCommandBuffer(m_device.get());
        m_device == nullptr || item == nullptr) {
      return {GPU_INIT_CMDBUF, SDL_GetError()};
    }
  }
  auto *commandBuf = m_commandBuffers[ECommandBufferRole::RENDER];

  getWindowSwapchainTexture(t_windowIn, m_windowTexture).mapError(logPassiveError);

  // Setup and start a render pass
  SDL_GPUColorTargetInfo targetInfo{.texture = m_windowTexture,
                                    .mip_level = 0,
                                    .layer_or_depth_plane = 0,
                                    .clear_color = NULL_BRUSH_COLOR,
                                    .load_op = SDL_GPU_LOADOP_CLEAR,
                                    .store_op = SDL_GPU_STOREOP_STORE,
                                    .cycle = false};
  m_renderPass = SDL_BeginGPURenderPass(commandBuf, &targetInfo, 1, nullptr);
  return {};
}
Error Graphics::endFrame(double &t_frameTimeOut) {
  SDL_EndGPURenderPass(m_renderPass);

  for (auto *item : m_commandBuffers) {
    if (auto err = SDL_SubmitGPUCommandBuffer(item); err) {
      return {FRAME_FINALIZE, SDL_GetError()};
    }
    item = {};
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

#define BAD_CALL(MESSAGE)                                                                          \
  return {                                                                                         \
    GET_WINDOW_SWAPCHAIN_TEXTURE,                                                                  \
        std::string_view("graphics::get_swapchain_texture called without a " MESSAGE)              \
  }

Error Graphics::getWindowSwapchainTexture(Window &t_windowIn, SDL_GPUTexture *t_textureOut) {
  auto *commandBuf = m_commandBuffers[ECommandBufferRole::RENDER];
  if (commandBuf == nullptr) {
    BAD_CALL("rendering command buffer");
  }
  if (t_textureOut == nullptr) {
    BAD_CALL("destination texture");
  }
  auto *window = t_windowIn.getRawHandle();
  if (window == nullptr) {
    BAD_CALL("subject window");
  }
  if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuf, window, &t_textureOut, nullptr, nullptr)) {
    return {GET_WINDOW_SWAPCHAIN_TEXTURE, SDL_GetError()};
  }
  return {};
}

#undef BAD_CALL

Error Graphics::uploadData(GPUBuffer &t_buf, void *t_data, uint32_t t_size, bool t_cycle) {
  void *mappedBuffer = SDL_MapGPUTransferBuffer(m_device.get(), m_uploadBuffer, t_cycle);
  memcpy(mappedBuffer, t_data, t_size);
  SDL_UnmapGPUTransferBuffer(m_device.get(), m_uploadBuffer);

  SDL_GPUTransferBufferLocation source = {.transfer_buffer = m_uploadBuffer, .offset = 0};
  SDL_GPUBufferRegion destination = {.buffer = t_buf.m_buffer, .offset = 0, .size = t_size};
  SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(m_commandBuffers[ECommandBufferRole::COPY]);
  SDL_UploadToGPUBuffer(copyPass, &source, &destination, t_cycle);
  SDL_EndGPUCopyPass(copyPass);

  return {};
}
