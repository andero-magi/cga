#include "cga_window.h"
#include "GLFW/glfw3.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "stdlib.h"

#define ERRORLOG(msg) fprintf_s(stderr, "[ERROR] %s\n", msg)

typedef GLFWwindow* window_t;

static key_callback_t callback = NULL;
static window_t win;
static frame_callback_t frameCallback;

static double lastStart = 0;
static float deltaTime = 1;
static float ratio = 1;
static int width = 640;
static int height = 480;
static char* title = "A C Game attempt.";
static int bVsyncState = 1;

static int frameCounter = 0;
static float frameActiveTime = 0.0f;

static void onError(int error, const char* desc) {
  printf("[ERROR] %s\n", desc);
}

static void onKeyCallback(window_t window, int key, int scancode, int action, int mods) {
  if (!callback) {
    return;
  }

  callback(key, action, mods);
}

int cgaInit() {
  window_t window;
  glfwSetErrorCallback(onError);

  if (!glfwInit()) {
    ERRORLOG("Failed to initialize GLFW");
    return 0;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  window = glfwCreateWindow(640, 480, "C Game Attempt", NULL, NULL);

  if (window == NULL) {
    glfwTerminate();
    ERRORLOG("Window creation failed, exiting");
    return 0;
  }

  glfwSetKeyCallback(window, onKeyCallback);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(bVsyncState);

  win = window;

  return 1;
}

void cgaLoop() {
  while (!glfwWindowShouldClose(win)) {
    glfwGetFramebufferSize(win, &width, &height);
    ratio = width / (float) height;

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    double start = glfwGetTime();

    deltaTime = (float) (start - lastStart);
    lastStart = start;

    frameCounter++;
    frameActiveTime += deltaTime;

    if (frameCallback != NULL) {
      frameCallback(deltaTime, ratio);
    }

    glfwSwapBuffers(win);
    glfwPollEvents();
  }
}

void cgaClose() {
  glfwDestroyWindow(win);
  glfwTerminate();
}

void cgaSetKeyCallback(key_callback_t callbackfn) {
  callback = callbackfn;
}

void cgaSetShouldClose(int bState) {
  if (win == NULL) {
    return;
  }

  glfwSetWindowShouldClose(win, bState);
}

void cgaSetFrameCallback(frame_callback_t callbackfn) {
  frameCallback = callbackfn;
}

void cgaSetScreenSize(int w, int h) {
  width = w;
  height = h;
  ratio = width / (float) height;

  if (win != NULL) {
    glfwSetWindowSize(win, width, height);
  }
}

void cgaGetScreenSize(int* w, int* h) {
  if (win == NULL) {
    *w = width;
    *h = height;

    return;
  }

  glfwGetWindowSize(win, w, h);
}

void cgaSetScreenTitle(const char* t) {
  if (!t) {
    return;
  }

  title = t;

  if (win != NULL) {
    glfwSetWindowTitle(win, t);
  }
}

float cgaGetScreenRatio() {
  return ratio;
}


float cgaGetFps() {
  return frameCounter / frameActiveTime;
}

float cgaGetActiveTime() {
  return frameActiveTime;
}

float cgaGetDeltaTime() {
  return deltaTime;  
}

int cgaGetFrameCounter() {
  return frameCounter;
}

void cgaSetVsync(int bState) {
  bVsyncState = bState;

  if (win != NULL) {
    glfwSwapInterval(bState);
  }
}
