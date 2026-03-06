#pragma once
#include <SDL3/SDL_video.h>

struct Window
{
    SDL_Window handle;
    int w, h;
    bool resized, focused, shouldquit;
};
