#pragma once
#include "Subsystem.h"
#include "Window.h"
#include <SDL3/SDL_stdinc.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

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

struct GPU_Buffer {
  SDL_GPUDevice *m_device;
  SDL_GPUBuffer *m_sdl_handle;
  SDL_GPUBufferUsageFlags m_usage;
  Uint32 m_size;

  GPU_Buffer(SDL_GPUDevice *device, SDL_GPUBufferUsageFlags usage, Uint32 size);
  ~GPU_Buffer();
};

struct GPU_Texture {
  SDL_GPUDevice *m_device;
  SDL_GPUTexture *m_sdl_handle;
  SDL_GPUTextureFormat m_format;
  Uint32 m_width, m_height;

  GPU_Texture(SDL_GPUDevice *device, Uint32 width, Uint32 height);
  ~GPU_Texture();
};

struct GPU_Shader {
  SDL_GPUDevice *m_device;
  SDL_GPUShader *m_sdl_handle;
  SDL_GPUShaderStage m_stage;
  SDL_GPUShaderFormat m_format;
  std::string m_filepath;
  // Uint8 *m_bytecode;

  GPU_Shader(SDL_GPUDevice *device, SDL_GPUShaderStage stage, SDL_GPUShaderFormat format,
             std::string filepath);
  ~GPU_Shader();
};

// TODO: create a GPU_Pipeline struct

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

  // NOTE: this does not do anything like size checks or grouping upload commands
  // or handling data larger then it's buffer in multiple uploads
  Error uploadData(GPU_Buffer &buffer, void *data, Uint32 size, bool cycle);
  // Error uploadData(GPU_Buffer &buffer, void *data, Uint32 size, bool cycle);

private:
  enum ECommandBufferRole : uint8_t { RENDER = 0, COPY = 1, LENGTH };
  // This is only a raw pointer because we explicitly do not free it per docs and we do not benefit
  // from ownership semantics at this point
  std::array<SDL_GPUCommandBuffer *, ECommandBufferRole::LENGTH> m_commandBuffers{};
  std::shared_ptr<SDL_GPUDevice> m_device;
  SDL_GPUTransferBuffer *m_upload_buffer;
  SDL_GPUTransferBuffer *m_download_buffer;

  uint64_t m_lastFrameTime{};
  // Transient handle that only lives between beginFrame and endFrame; nullptr all other times
  SDL_GPURenderPass *m_renderPass{};
  DEFINE_PROPERTY(double, m_frameIntervalNS, getFrameInterval, setFrameInterval, {});
};
