#pragma once

namespace graphics {
enum class EError : uint8_t {
  SUBSYSTEM_INIT = 0,
  GPU_INIT,
  GPU_INIT_CMDBUF,
  GPU_WINDOW_CLAIM,
  SET_SWAPCHAIN_PARAMS,
  GET_WINDOW_SWAPCHAIN_TEXTURE,
};

ERROR_CONTEXT_TYPE({
case SUBSYSTEM_INIT:
  return "initializing the graphics subsystem";
case GPU_INIT:
  return "initializing the GPU device";
case GPU_INIT_CMDBUF:
  return "acquiring a command buffer for the GPU device";
case GPU_WINDOW_CLAIM:
  return "the GPU device was claiming its window";
case SET_SWAPCHAIN_PARAMS:
  return "setting parameters for the swapchain";
case GET_WINDOW_SWAPCHAIN_TEXTURE:
  return "getting the texture from the swapchain";
})

Error init(Graphics &ctx);
Error attach_window(Graphics &ctx, Window &window);
Error get_window_swapchain_texture(Graphics &ctx, Window &window, SDL_GPUTexture *output);
} // namespace graphics

struct Graphics {
  std::shared_ptr<SDL_GPUDevice> m_handle;
  std::shared_ptr<SDL_GPUCommandBuffer> m_main_command_buffer;
};

using GraphicsError = graphics::Error;
