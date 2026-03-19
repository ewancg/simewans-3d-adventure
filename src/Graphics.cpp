#include "Graphics.h"
#include "common.h"
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>
#include <cstring>
#include <memory>
using enum EGraphicsError;
using Error = GraphicsError;

GPU_Buffer::GPU_Buffer(SDL_GPUDevice *device, SDL_GPUBufferUsageFlags usage, Uint32 size)
    : m_device(device), m_usage(usage), m_size(size) {
  SDL_GPUBufferCreateInfo create_info = {.usage = usage, .size = size, .props = 0};
  m_sdl_handle = SDL_CreateGPUBuffer(device, &create_info);
}
GPU_Buffer::~GPU_Buffer() {
  if (m_sdl_handle != nullptr) {
    SDL_ReleaseGPUBuffer(m_device, m_sdl_handle);
  }
}

GPU_Texture::GPU_Texture(SDL_GPUDevice *device, Uint32 width, Uint32 height)
    : m_device(device), m_width(width), m_height(height) {
  SDL_GPUTextureCreateInfo create_info = {.type = SDL_GPU_TEXTURETYPE_2D,
                                          .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UINT,
                                          .usage = 0,
                                          .width = width,
                                          .height = height,
                                          .layer_count_or_depth = 0,
                                          .num_levels = 0,
                                          .sample_count = SDL_GPU_SAMPLECOUNT_1,
                                          .props = 0};
  m_sdl_handle = SDL_CreateGPUTexture(device, &create_info);
}
GPU_Texture::~GPU_Texture() {
  if (m_sdl_handle != nullptr) {
    SDL_ReleaseGPUTexture(m_device, m_sdl_handle);
  }
}

GPU_Shader::GPU_Shader(SDL_GPUDevice *device, SDL_GPUShaderStage stage, SDL_GPUShaderFormat format,
                       std::string filepath)
    : m_device(device), m_stage(stage), m_format(format), m_filepath(filepath) {
  // read in the shader byte code
  size_t code_size;
  Uint8 *bytecode = reinterpret_cast<Uint8 *>(SDL_LoadFile(filepath.c_str(), &code_size));

  // create gpu shader
  SDL_GPUShaderCreateInfo create_info = {.code_size = code_size,
                                         .code = bytecode,
                                         .entrypoint = "main",
                                         .format = format,
                                         .stage = stage,
                                         .num_samplers = 0,
                                         .num_storage_textures = 0,
                                         .num_storage_buffers = 0,
                                         .num_uniform_buffers = 0,
                                         .props = 0};
  SDL_CreateGPUShader(device, &create_info);

  SDL_free(bytecode);
}
GPU_Shader::~GPU_Shader() {
  if (m_sdl_handle != nullptr) {
    SDL_ReleaseGPUShader(m_device, m_sdl_handle);
  }
}

Error Graphics::onInit() {
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

  m_device = std::shared_ptr<SDL_GPUDevice>(device, SDL_DestroyGPUDevice);

  /* CREATE SDL GPU TRANSFER BUFFERS */
  auto release_transfer_buffer_lambda = [device](SDL_GPUTransferBuffer *buffer) {
    SDL_ReleaseGPUTransferBuffer(device, buffer);
  };

  SDL_GPUTransferBufferCreateInfo upload_create_info = {
      .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = 1024 * 1024, .props = 0};
  m_upload_buffer = SDL_CreateGPUTransferBuffer(device, &upload_create_info);
  if (m_upload_buffer == nullptr) {
    return {GPU_INIT, SDL_GetError()};
  }

  SDL_GPUTransferBufferCreateInfo download_create_info = {
      .usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD, .size = 1024 * 1024, .props = 0};
  m_download_buffer = SDL_CreateGPUTransferBuffer(device, &download_create_info);
  if (m_download_buffer == nullptr) {
    return {GPU_INIT, SDL_GetError()};
  }

  return {};
}

Error Graphics::onDestroy() {
  if (m_upload_buffer != nullptr) {
    SDL_ReleaseGPUTransferBuffer(m_device.get(), m_upload_buffer);
  }
  if (m_download_buffer != nullptr) {
    SDL_ReleaseGPUTransferBuffer(m_device.get(), m_download_buffer);
  }
  if (m_device) {
    SDL_DestroyGPUDevice(m_device.get());
  }
  // Explicitly do not free command buffers
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
  return {};
}

Error Graphics::onUpdate() { return {}; }

// ----------

Error Graphics::beginFrame(SDL_GPUTexture *t_textureOut) {
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
  SDL_GPUColorTargetInfo targetInfo{.texture = t_textureOut,
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

Error Graphics::uploadData(GPU_Buffer &buffer, void *data, Uint32 size, bool cycle) {
  void *mapped_buffer = SDL_MapGPUTransferBuffer(m_device.get(), m_upload_buffer, cycle);
  memcpy(mapped_buffer, data, size);
  SDL_UnmapGPUTransferBuffer(m_device.get(), m_upload_buffer);

  SDL_GPUTransferBufferLocation source = {.transfer_buffer = m_upload_buffer, .offset = 0};
  SDL_GPUBufferRegion destination = {.buffer = buffer.m_sdl_handle, .offset = 0, .size = size};
  SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(m_commandBuffers[ECommandBufferRole::COPY]);
  SDL_UploadToGPUBuffer(copy_pass, &source, &destination, cycle);
  SDL_EndGPUCopyPass(copy_pass);

  return {};
}
