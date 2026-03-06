#include <string.h>
#include <stdio.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>
#include "input.h"

bool input_state[inputTotal] = {0};
bool prev_input_state[inputTotal] = {0};

MouseData mdata;
MouseData *get_mouse_data(){return &mdata;}

void set_mouse_pos(Window *screen, int x, int y)
{
    SDL_WarpMouseInWindow((SDL_Window *)screen->windowdata, x, y);
}
void center_mouse_pos(Window *screen)
{
    SDL_WarpMouseInWindow((SDL_Window *)screen->windowdata, screen->w/2, screen->h/2);
}

//checks to see if the input is down
bool input_down(Input in)
{
    return input_state[in];
}
//checks to see if the input was pressed
bool input_pressed(Input in)
{
    return input_state[in] == true && prev_input_state[in] == false;
}

bool get_key_state (SDL_KeyboardEvent event)
{
    return event.down == true || event.repeat != true;
};

static void handle_keyboard_event(SDL_KeyboardEvent event)
{
    switch (event.scancode)
    {
    case(SDL_SCANCODE_A):
        input_state[keyA] = get_key_state(event);
        break;
    case(SDL_SCANCODE_B):
        input_state[keyB] = get_key_state(event);
        break;
    case(SDL_SCANCODE_C):
        input_state[keyC] = get_key_state(event);
        break;
    case(SDL_SCANCODE_D):
        input_state[keyD] = get_key_state(event);
        break;
    case(SDL_SCANCODE_E):
        input_state[keyE] = get_key_state(event);
        break;
    case(SDL_SCANCODE_F):
        input_state[keyF] = get_key_state(event);
        break;
    case(SDL_SCANCODE_G):
        input_state[keyG] = get_key_state(event);
        break;
    case(SDL_SCANCODE_H):
        input_state[keyH] = get_key_state(event);
        break;
    case(SDL_SCANCODE_I):
        input_state[keyI] = get_key_state(event);
        break;
    case(SDL_SCANCODE_J):
        input_state[keyJ] = get_key_state(event);
        break;
    case(SDL_SCANCODE_K):
        input_state[keyK] = get_key_state(event);
        break;
    case(SDL_SCANCODE_L):
        input_state[keyL] = get_key_state(event);
        break;
    case(SDL_SCANCODE_M):
        input_state[keyM] = get_key_state(event);
        break;
    case(SDL_SCANCODE_N):
        input_state[keyN] = get_key_state(event);
        break;
    case(SDL_SCANCODE_O):
        input_state[keyO] = get_key_state(event);
        break;
    case(SDL_SCANCODE_P):
        input_state[keyP] = get_key_state(event);
        break;
    case(SDL_SCANCODE_Q):
        input_state[keyQ] = get_key_state(event);
        break;
    case(SDL_SCANCODE_R):
        input_state[keyR] = get_key_state(event);
        break;
    case(SDL_SCANCODE_S):
        input_state[keyS] = get_key_state(event);
        break;
    case(SDL_SCANCODE_T):
        input_state[keyT] = get_key_state(event);
        break;
    case(SDL_SCANCODE_U):
        input_state[keyU] = get_key_state(event);
        break;
    case(SDL_SCANCODE_V):
        input_state[keyV] = get_key_state(event);
        break;
    case(SDL_SCANCODE_W):
        input_state[keyW] = get_key_state(event);
        break;
    case(SDL_SCANCODE_X):
        input_state[keyX] = get_key_state(event);
        break;
    case(SDL_SCANCODE_Y):
        input_state[keyY] = get_key_state(event);
        break;
    case(SDL_SCANCODE_Z):
        input_state[keyZ] = get_key_state(event);
        break;

    case(SDL_SCANCODE_0):
        input_state[key0] = get_key_state(event);
        break;
    case(SDL_SCANCODE_1):
        input_state[key1] = get_key_state(event);
        break;
    case(SDL_SCANCODE_2):
        input_state[key2] = get_key_state(event);
        break;
    case(SDL_SCANCODE_3):
        input_state[key3] = get_key_state(event);
        break;
    case(SDL_SCANCODE_4):
        input_state[key4] = get_key_state(event);
        break;
    case(SDL_SCANCODE_5):
        input_state[key5] = get_key_state(event);
        break;
    case(SDL_SCANCODE_6):
        input_state[key6] = get_key_state(event);
        break;
    case(SDL_SCANCODE_7):
        input_state[key7] = get_key_state(event);
        break;
    case(SDL_SCANCODE_8):
        input_state[key8] = get_key_state(event);
        break;
    case(SDL_SCANCODE_9):
        input_state[key9] = get_key_state(event);
        break;

    case(SDL_SCANCODE_LSHIFT):
        input_state[keyLeftShift] = get_key_state(event);
        break;
    case(SDL_SCANCODE_LCTRL):
        input_state[keyLeftCtrl] = get_key_state(event);
        break;
    case(SDL_SCANCODE_LALT):
        input_state[keyLeftAlt] = get_key_state(event);
        break;
    case(SDL_SCANCODE_RSHIFT):
        input_state[keyRightShift] = get_key_state(event);
        break;
    case(SDL_SCANCODE_RCTRL):
        input_state[keyRightCtrl] = get_key_state(event);
        break;
    case(SDL_SCANCODE_RALT):
        input_state[keyRightAlt] = get_key_state(event);
        break;

    case(SDL_SCANCODE_RETURN):
        input_state[keyEnter] = get_key_state(event);
        break;
    case(SDL_SCANCODE_SPACE):
        input_state[keySpace] = get_key_state(event);
        break;
    case(SDL_SCANCODE_ESCAPE):
        input_state[keyEscape] = get_key_state(event);
        break;

    case(SDL_SCANCODE_LEFT):
        input_state[keyLeft] = get_key_state(event);
        break;
    case(SDL_SCANCODE_RIGHT):
        input_state[keyRight] = get_key_state(event);
        break;
    case(SDL_SCANCODE_UP):
        input_state[keyUp] = get_key_state(event);
        break;
    case(SDL_SCANCODE_DOWN):
        input_state[keyDown] = get_key_state(event);
        break;
    default:
        break;
    }
    return;
}

void handle_window_event(SDL_WindowEvent event, Window *screen)
{
    switch(event.type)
    {
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        screen->focused = false;
        break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        screen->focused = true;
        break;
    case SDL_EVENT_WINDOW_RESIZED:
        screen->w = event.data1;
        screen->h = event.data2;
        //glViewport(0, 0, screen->w, screen->h);
        screen->resized = true;
        break;
    default:
        break;
    }
}

void handle_mouse_button_event(SDL_MouseButtonEvent event)
{
    switch(event.button)
    {
    case SDL_BUTTON_LEFT:
        input_state[mouseLeft] = event.down;
        break;
    case SDL_BUTTON_MIDDLE:
        input_state[mouseMiddle] = event.down;
        break;
    case SDL_BUTTON_RIGHT:
        input_state[mouseRight] = event.down;
        break;
    case SDL_BUTTON_X1:
        break;
    case SDL_BUTTON_X2:
        break;
    }
}

void update_input(Window *screen)
{
    memcpy(prev_input_state, input_state, inputTotal);
	
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_EVENT_QUIT)
            screen->shouldquit = true;
        else if(event.type >= SDL_EVENT_WINDOW_SHOWN && event.type <= SDL_EVENT_KEY_DOWN)
            handle_window_event(event.window, screen);
        else if(event.type >= SDL_EVENT_KEY_DOWN && event.type <= SDL_EVENT_MOUSE_MOTION)
            handle_keyboard_event(event.key);
        else if(event.type >= SDL_EVENT_MOUSE_MOTION && event.type <= SDL_EVENT_JOYSTICK_AXIS_MOTION)
        {
            const SDL_DisplayMode *dm = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow((SDL_Window *)screen->windowdata));

            mdata.x = (float)event.motion.x/(float)dm->w;
            mdata.y = (float)event.motion.y/(float)dm->h;
            mdata.xm = (float)event.motion.xrel/(float)dm->w;
            mdata.ym = (float)event.motion.yrel/(float)dm->h;

            if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP)
                handle_mouse_button_event(event.button);
            if(event.type == SDL_EVENT_MOUSE_WHEEL)
            {
                if(event.wheel.y < 0)
                    input_state[mouseWheelDown] = true;
                if(event.wheel.y > 0)
                    input_state[mouseWheelUp] = true;
            }
        }
    }

    return;
}

void input_quit()
{

    return;
}