class GPUTexture {
  SDL_GPUDevice *m_device{};
  SDL_GPUTexture *m_texture{};
  SDL_GPUTextureFormat m_format{};
  uint32_t m_width{}, m_height{};

public:
  friend class Graphics;

  NO_COPY_MOVE_OR_ASSIGN(GPUTexture, "cannot copy textures, they are unique handles",
                         "cannot move textures")

  GPUTexture(SDL_GPUDevice *t_device, uint32_t t_width, uint32_t t_height)
      : m_device(t_device), m_width(t_width), m_height(t_height) {
    SDL_GPUTextureCreateInfo createInfo = {.type = SDL_GPU_TEXTURETYPE_2D,
                                           .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UINT,
                                           .usage = 0,
                                           .width = t_width,
                                           .height = t_height,
                                           .layer_count_or_depth = 0,
                                           .num_levels = 0,
                                           .sample_count = SDL_GPU_SAMPLECOUNT_1,
                                           .props = 0};
    m_texture = SDL_CreateGPUTexture(t_device, &createInfo);
  }
  ~GPUTexture() {
    if (m_texture != nullptr) {
      SDL_ReleaseGPUTexture(m_device, m_texture);
    }
  }
};
