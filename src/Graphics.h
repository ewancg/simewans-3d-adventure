#pragma once
#include "Subsystem.h"
#include "Window.h"

#ifndef GRAPHICS_DEBUGGING
#define GRAPHICS_DEBUGGING false
#endif

#define ERROR_ENTRIES(E)                                                                           \
  E(INIT, "initializing the graphics subsystem")                                                   \
  E(GPU_INIT, "initializing the GPU device")                                                       \
  E(GPU_INIT_CMDBUF, "acquiring a command buffer for the GPU device")                              \
  E(GPU_WINDOW_CLAIM, "the GPU device was claiming its window")                                    \
  E(SET_SWAPCHAIN_PARAMS, "setting parameters for the swapchain")                                  \
  E(GET_WINDOW_SWAPCHAIN_TEXTURE, "getting the texture from the swapchain")                        \
  E(FRAME_BEGIN, "beginning a new frame")                                                          \
  E(FRAME_FINALIZE, "finishing a frame")
DEFINE_DERIVED_ERROR_TYPES(Graphics, Subsystem, ERROR_ENTRIES);

#undef ERROR_ENTRIES

class Graphics : public Subsystem<GraphicsError> {
  SUBSYSTEM(Graphics)
public:
  /// Begins render pass
  Error beginFrame(SDL_GPUTexture *t_textureOut);
  /// Ends render pass, submits command buffer, returns frame time
  Error endFrame(double &t_frameTimeOut);

  /// Hooks the window up to the graphics driver and creates a swapchain for it
  Error attachWindow(Window &t_windowIn);
  /// Acquires the swapchain texture for direct rendering
  Error getWindowSwapchainTexture(Window &t_windowIn, SDL_GPUTexture *t_textureOut);

private:
  enum ECommandBufferRole : uint8_t { RENDER = 0, LENGTH };
  // This is only a raw pointer because we explicitly do not free it per docs and we do not benefit
  // from ownership semantics at this point
  std::array<SDL_GPUCommandBuffer *, ECommandBufferRole::LENGTH> m_commandBuffers{};

  SDL_GPUDevice *m_device{};

  uint64_t m_lastFrameTime{};
  // Transient handle that only lives between beginFrame and endFrame; nullptr all other times
  SDL_GPURenderPass *m_renderPass{};
  DEFINE_PROPERTY(double, m_frameIntervalNS, getFrameInterval, setFrameInterval, {});
};
