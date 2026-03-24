struct MainConfigData {
  SDL_FColor null_brush_color{
      .r = .625f * 0xe2, .g = .625f * 0x43, .b = .625f * 0x30, .a = .625f * 0xFF};
};
struct GraphicsConfigData {
  bool     vsync;
  float    dpi_override;
  uint16_t fps_cap;
};
