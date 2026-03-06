#pragma once
#include "graphics.h"

enum Input
{
    keyA,    keyB,    keyC,    keyD,    keyE,    keyF,    keyG,    keyH,
    keyI,    keyJ,    keyK,    keyL,    keyM,    keyN,    keyO,    keyP,
    keyQ,    keyR,    keyS,    keyT,    keyU,    keyV,    keyW,    keyX,
    keyY,    keyZ,

    key0,    key1,    key2,    key3,    key4,    key5,    key6,    key7,    key8,    key9,

    keyLeftShift, keyLeftCtrl, keyLeftAlt, keyRightShift, keyRightCtrl, keyRightAlt, keyEnter, keySpace,
    keyEscape,
    keyLeft,    keyRight,    keyUp,    keyDown,

    mouseLeft,    mouseRight,    mouseMiddle, mouseWheelDown, mouseWheelUp,

    inputTotal
};

struct MouseData
{
    float x, y;
    float xm, ym;
};
MouseData *get_mouse_data();

void set_mouse_pos(Window *screen, int x, int y);
void center_mouse_pos(Window *screen);

void set_input_key(Input in, char key);

//checks to see if the input
bool input_down(Input in);
//checks to see if the input was pressed
bool input_pressed(Input in);

//int input_initialize(Window *screen);
void update_input(Window *screen);
void input_quit();
