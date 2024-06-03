#include "cga_window.h"
#include "glfw/glfw3.h"
#include "stdio.h"
#include <time.h>
#include "stdlib.h"

typedef GLFWwindow* window_t;

static key_callback_t callback = NULL;
static window_t win;
static frame_callback_t frameCallback;

static double lastStart = 0;
static float deltaTime = 1;

static void onError(int error, const char* desc) {
  printf("Error!\n  %s\n", desc);
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
    printf("Error:\n  Failed to initialize GLFW\n");
    return 0;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  window = glfwCreateWindow(640, 480, "C Game Attempt", NULL, NULL);

  if (window == NULL) {
    glfwTerminate();
    printf("Error:\n  Window creation failed, exiting\n");
    return 0;
  }

  glfwSetKeyCallback(window, onKeyCallback);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  win = window;

  return 1;
}

void cgaLoop() {
  while (!glfwWindowShouldClose(win)) {
    float ratio = 1;
    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(win, &width, &height);

    ratio = width / (float) height;

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    double start = glfwGetTime();
    float deltaTime = (float) (start - lastStart);

    lastStart = start;

    if (frameCallback != NULL) {
      frameCallback(deltaTime);
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