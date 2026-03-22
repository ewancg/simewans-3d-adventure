#define DEFAULT_TRANSFER_BUFFER_SIZE (1024 * 1024)

class GPUBuffer {
  SDL_GPUDevice *m_device{};
  SDL_GPUBuffer *m_buffer{};
  SDL_GPUBufferUsageFlags m_usageFlags{};
  uint32_t m_size{};

public:
  NO_COPY_MOVE_OR_ASSIGN(GPUBuffer, "cannot copy buffers, they are unique handles",
                         "cannot move buffers")
  friend class Graphics;
  GPUBuffer(SDL_GPUDevice *t_device, SDL_GPUBufferUsageFlags t_usage, uint32_t t_size)
      : m_device(t_device), m_usageFlags(t_usage), m_size(t_size) {
    SDL_GPUBufferCreateInfo createInfo = {.usage = t_usage, .size = t_size, .props = 0};
    m_buffer = SDL_CreateGPUBuffer(t_device, &createInfo);
  }
  ~GPUBuffer() {
    if (m_buffer != nullptr) {
      SDL_ReleaseGPUBuffer(m_device, m_buffer);
    }
  }
};
