#pragma once

struct Window {
  SDL_Window handle;
  int w, h;
  bool resized, focused, shouldquit;
};
