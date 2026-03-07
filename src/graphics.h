#pragma once
#include "window.h"

#define GRAPHICS_ERRORS(E)                                                                         \
  E(SUBSYSTEM_INIT, "initializing the graphics subsystem")                                         \
  E(GPU_INIT, "initializing the GPU device")                                                       \
  E(GPU_INIT_CMDBUF, "acquiring a command buffer for the GPU device")                              \
  E(GPU_WINDOW_CLAIM, "the GPU device was claiming its window")                                    \
  E(SET_SWAPCHAIN_PARAMS, "setting parameters for the swapchain")                                  \
  E(GET_WINDOW_SWAPCHAIN_TEXTURE, "getting the texture from the swapchain")                        \
  E(FRAME_BEGIN, "beginning a new frame")                                                          \
  E(FRAME_FINALIZE, "finishing a frame")
DEFINE_ERROR_TYPES(Graphics, GRAPHICS_ERRORS);
#undef ERRORS

class Graphics : public Subsystem<GraphicsError> {
  using Error = GraphicsError;
  enum ECommandBufferRole : uint8_t { RENDER = 0, LENGTH };
  // This is only a raw pointer because we explicitly do not free it per docs and we do not benefit
  // from ownership semantics at this point
  std::array<SDL_GPUCommandBuffer *, ECommandBufferRole::LENGTH> m_command_buffers{};
  std::shared_ptr<SDL_GPUDevice> m_device;

  uint64_t m_last_time{};
  // Transient handle that only lives between beginFrame and endFrame; nullptr all other times
  SDL_GPURenderPass *m_render_pass{};

public:
  Error onInit();
  Error onDestroy();

  /// Begins render pass
  Error beginFrame(SDL_GPUTexture *tex);
  /// Ends render pass, submits command buffer, returns frame time
  double endFrame(uint64_t frame_interval);

  /// Hooks the window up to the graphics driver and creates a swapchain for it
  Error attachWindow(Window &window);
  /// Acquires the swapchain texture for direct rendering
  Error getWindowSwapchainTexture(Window &window, SDL_GPUTexture *output);
};
