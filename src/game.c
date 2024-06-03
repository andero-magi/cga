#include "game.h"
#include "cga_core.h"
#include "cga_window.h"
#include "cga_inputs.h"
#include <stdio.h>

#define BOARD_WIDTH 8
#define BOARD_HEIGHT 8
#define BOARD_SIZE (BOARD_HEIGHT * BOARD_WIDTH)
#define TO_INDEX(x, y) ((BOARD_WIDTH * y) + x)
#define MOVE_TIME_SECS 0.5

#define NO_CELL_VALUE 0

typedef enum {
  X,
  Y
} axis_t;

typedef enum {
  GS_INACTIVE,
  GS_ACTIVE,
  GS_PAUSED,
} game_state_t;

typedef enum {
  DIR_UP,
  DIR_DOWN,
  DIR_LEFT,
  DIR_RIGHT
} shift_direction_t;

static game_state_t gameState = GS_INACTIVE;
static int board[BOARD_SIZE];
static float gameTime;

static void resetBoard() {
  for (int i = 0; i < BOARD_SIZE; i++) {
    board[i] = NO_CELL_VALUE;
  }
}

static int getCell(int x, int y) {
  int index = TO_INDEX(x, y);
  return board[index];
}

static void setCell(int x, int y, int value) {
  int index = TO_INDEX(x, y);

  if (value <= 0) {
    board[index] = NO_CELL_VALUE;
  } else {
    board[index] = value;
  }
}

static void shiftInDirection(shift_direction_t dir) {
  int startPos = 0;
  int moveDir;
  axis_t axis = X;

  
}

static void onInput(int key, int action, int mods) {
  if (action != ACTION_PRESS) {
    return;
  }

  if (key == KEY_ESCAPE) {
    cgaSetShouldClose(true);
    return;
  }

  if (gameState == GS_INACTIVE) {
    return;
  }

  switch (key) {
    case KEY_P:
    case KEY_SPACE:
      if (gameState == GS_PAUSED) {
        gameState = GS_ACTIVE;
      } else {
        gameState = GS_PAUSED;
      }
      return;

    case KEY_UP:
    case KEY_W:
      shiftInDirection(DIR_UP);
      return;

    case KEY_DOWN:
    case KEY_S:
      shiftInDirection(DIR_DOWN);
      return;

    case KEY_LEFT:
    case KEY_A:
      shiftInDirection(DIR_LEFT);
      return;

    case KEY_RIGHT:
    case KEY_D:
      shiftInDirection(DIR_RIGHT);
      return;

    default:
      break;
  }
}

void onUpdate(float deltaTime) {
  gameTime += deltaTime;
}

void gameMain() {
  if (!cgaInit()) {
    return;
  }

  cgaSetKeyCallback(onInput);
  cgaSetFrameCallback(onUpdate);

  cgaLoop();
  cgaClose();
}