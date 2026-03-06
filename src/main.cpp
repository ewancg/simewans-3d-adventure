#include <print>
#include <stdio.h>
#include <SDL3/SDL3.h>
#include "input.h"
#include "graphics.h"
#ifndef UNIT_TESTING

//Start and End Frame Functions
void start_frame(Window *screen)
{
    update_input(screen);
    //screenClear();
    return;
}

float end_frame(uint64_t framewait, Window *screen)
{
    static uint64_t past;
    //screenUpdate(screen);

    uint64_t now = SDL_GetTicksNS();
    uint64_t frametime = now - past;
    //printf("%u\n", frametime.count());

    SDL_DelayPrecise(framewait-frametime);

    past = now;
    return (float)framewait/1000000000.0;
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    Window window;
    window.handle = NULL;
    window.w = 960;
    window.h = 540;
    window.resized = false;
    window.focused = true;
    window.shouldquit = false;
    window.handle = SDL_CreateWindow("AwesomeSauce", 960, 540,  | SDL_WINDOW_RESIZABLE);

    while(window.shouldquit == false)
    {
        start_frame(&window);
        end_frame(16666667, &window);   //16666667 is the nanoseconds per frame at 60 fps
    }
}

#else
#include <catch2/catch_test_macros.hpp>
// TEST_CASE("unit test", "") { REQUIRE(true); }
#endif
