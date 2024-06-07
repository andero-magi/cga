#ifndef CGA_WINDOW_H
#define CGA_WINDOW_H

#include "cga_core.h"

typedef void (*key_callback_t)(int key, int action, int mods);
typedef void (*frame_callback_t)(float deltaTime, float ratio);

int cgaInit();

void cgaLoop();

void cgaClose();

void cgaSetKeyCallback(key_callback_t callbackfn);

void cgaSetShouldClose(int bState);

void cgaSetFrameCallback(frame_callback_t callbackfn);

void cgaSetScreenSize(int width, int height);

void cgaGetScreenSize(int* width, int* height);

void cgaSetScreenTitle(const char* title);

float cgaGetScreenRatio();

float cgaGetFps();

float cgaGetActiveTime();

float cgaGetDeltaTime();

int cgaGetFrameCounter();

void cgaSetVsync(boolean bState);

#endif // CGA_WINDOW_H