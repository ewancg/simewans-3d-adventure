struct MainConfigData {
  bool force_synchronous_events{};

  SDL_FColor null_brush_color{
      .r = .625f * 0xe2, .g = .625f * 0x43, .b = .625f * 0x30, .a = .625f * 0xFF};
};
struct GraphicsConfigData {
  bool     vsync{true};
  float    dpi_override{0};
  uint16_t fps_cap{0};
};
