#ifndef CGA_WINDOW_H
#define CGA_WINDOW_H

#include "cga_core.h"

typedef void (*key_callback_t)(int key, int action, int mods);
typedef void (*frame_callback_t)(float deltaTime);

int cgaInit();

void cgaLoop();

void cgaClose();

void cgaSetKeyCallback(key_callback_t callbackfn);

void cgaSetShouldClose(int bState);

void cgaSetFrameCallback(frame_callback_t callbackfn);

#endif // CGA_WINDOW_H