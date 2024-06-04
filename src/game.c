#include "game.h"
#include "cga_core.h"
#include "cga_window.h"
#include "cga_inputs.h"
#include "glutil.h"
#include "font_draw.h"
#include <stdio.h>
#include <stdlib.h>
#include <D:/MinGW/include/GL/gl.h>

#define BOARD_WIDTH 4
#define BOARD_HEIGHT 4
#define BOARD_SIZE (BOARD_HEIGHT * BOARD_WIDTH)
#define MOVE_TIME_SECS 0.5
#define DEBUG_INFO_BUF_SIZE 100
#define TARGET_VALUE 2048.0f

#define TO_INDEX(x, y) ((BOARD_WIDTH * x) + y)
#define IN_BOUNDS(x, y) ((x >= 0 && x < BOARD_WIDTH) && (y >= 0 && y < BOARD_HEIGHT))
#define LERP(prog, a, b) (a + ((b - a) * prog))

#define NO_CELL_VALUE 0
#define STARTING_VALUE 2
#define OUT_OF_BOUNDS -1

int randomIndex = 0;
int randomSlotOrder[64] = {
  52, 33, 12,  9,  4, 59, 41, 50,  0, 51, 36, 60, 35, 27, 29, 10, 56, 47,
   7,  3, 14,  5, 40, 58, 34, 45,  8, 11, 20, 44, 32, 63, 39, 17, 25, 13, 
  24, 21,  6, 61,  2, 53, 37, 15, 38, 54, 18, 30, 26, 31,  1, 46, 49, 62, 
  28, 43, 55, 19, 23, 42, 57, 16, 48, 22
};

typedef enum {
  X,
  Y
} axis_t;

typedef enum {
  GS_INACTIVE,
  GS_ACTIVE,
  GS_PAUSED,
  GS_LOST
} game_state_t;

typedef enum {
  DIR_UP,
  DIR_DOWN,
  DIR_LEFT,
  DIR_RIGHT
} shift_direction_t;

static game_state_t gameState = GS_INACTIVE;
static int board[BOARD_SIZE] = {0};
static float gameTime = 0.0f;
static boolean debugInfoEnabled = 1;

static char* debugBuffer = NULL;

static int freeSlots[BOARD_SIZE] = {0};
static int freeSlotCount = 0;

static void findFreeSlots() {
  freeSlotCount = 0;
  
  for (int i = 0; i < BOARD_SIZE; i++) {
    int v = board[i];

    if (v != NO_CELL_VALUE) {
      continue;
    }

    freeSlots[freeSlotCount++] = i;
  }
}

static void resetBoard() {
  for (int i = 0; i < BOARD_SIZE; i++) {
    board[i] = NO_CELL_VALUE;
  }
}

static int getCell(int x, int y) {
  if (!IN_BOUNDS(x, y)) {
    return OUT_OF_BOUNDS;
  }

  int index = TO_INDEX(x, y);
  return board[index];
}

static void setCell(int x, int y, int value) {
  if (!IN_BOUNDS(x, y)) {
    return;
  }

  int index = TO_INDEX(x, y);

  int prevValue = board[index];

  if (value <= 0) {
    board[index] = NO_CELL_VALUE;
  } else {
    board[index] = value;
  }
}

static int nextRandomSlotIndex() {
  int n = randomIndex++;

  if (randomIndex >= BOARD_SIZE) {
    randomIndex = 0;
  }

  int slot = randomSlotOrder[n];
  return slot % BOARD_SIZE;
}

static boolean genRandomSlot();

static void startGame() {
  resetBoard();
  gameState = GS_ACTIVE;

  genRandomSlot();
  genRandomSlot();
}

static void setGameLost() {
  gameState = GS_LOST;
  printf("[DEBUG] Game lost!\n");
}

static boolean isGameLost() {
  for (int x = 0; x < BOARD_WIDTH; x++) {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      int v = getCell(x, y);

      if (v == NO_CELL_VALUE) {
        return false;
      }

      int b = getCell(x, y + 1);
      int a = getCell(x, y - 1);
      int l = getCell(x + 1, y);
      int r = getCell(x - 1, y);

      if (b == v || a == v || l == v || r == v) {
        return false;
      }
    }
  }

  return true;
}

static boolean genRandomSlot() {
  findFreeSlots();

  if (freeSlotCount == 0) {
    return false;
  }

  int rValue = rand();
  float local = rValue / (float) RAND_MAX;
  int index = (int) (local * freeSlotCount);

  int boardSlot = freeSlots[index];

  board[boardSlot] = STARTING_VALUE;
  return true;
}

static int shiftCell(int x, int y, int moveX, int moveY) {
  if (moveX == 0 && moveY == 0) {
    printf("[ERROR] Both move X and move Y are 0\n");
    return false;
  }

  int startVal = getCell(x, y);

  if (startVal == NO_CELL_VALUE) {
    return false;
  }

  setCell(x, y, NO_CELL_VALUE);

  int nextX = x;
  int nextY = y;

  int wasMoved = 0;

  while (1) {
    nextX = x + moveX;
    nextY = y + moveY;

    int v = getCell(nextX, nextY);

    if (v == OUT_OF_BOUNDS) {
      setCell(x, y, startVal);
      break;
    }

    if (v == NO_CELL_VALUE) {
      x = nextX;
      y = nextY;
      wasMoved = 1;

      continue;
    }

    if (v == startVal) {
      setCell(nextX, nextY, v + startVal);
      break;
    }

    setCell(x, y, startVal);
    break;
  }
}

static int shiftX(shift_direction_t dir) {
  int moveDir = 0;
  int start = 0;
  int shiftDir = 0;

  switch (dir) {
    case DIR_RIGHT:
      moveDir = 1;
      shiftDir = -1;
      start = 1;
      break;

    case DIR_LEFT:
      moveDir = -1;
      shiftDir = 1;
      start = BOARD_WIDTH - 1;
      break;

    default:
      return 0;
  }

  int moved = 0;

  for (int y = 0; y < BOARD_HEIGHT; y++) {
    for (int x = start; x >= 0 && x < BOARD_WIDTH; x += moveDir) {
      moved += shiftCell(x, y, shiftDir, 0);
    }
  }

  return moved;
}

static int shiftY(shift_direction_t dir) {
  int moveDir = 0;
  int start = 0;
  int shiftDir = 0;

  switch (dir) {
    case DIR_DOWN:
      shiftDir = 1;
      moveDir = -1;
      start = BOARD_HEIGHT - 1;
      break;
    
    case DIR_UP:
      shiftDir = -1;
      moveDir = 1;
      start = 1;
      break;

    default:
      return 0;
  }

  int moved = 0;

  for (int x = 0; x < BOARD_WIDTH; x++) {
    for (int y = start; y >= 0 && y < BOARD_HEIGHT; y += moveDir) {
      moved += shiftCell(x, y, 0, shiftDir);
    }
  }

  return moved;
}

static void shiftInDirection(shift_direction_t dir) {
  int movedPieces = 0;

  if (dir == DIR_DOWN || dir == DIR_UP) {
    movedPieces = shiftY(dir);
  } else {
    movedPieces = shiftX(dir);
  }

  if (movedPieces == 0) {
    return;
  }

  genRandomSlot();

  if (isGameLost()) {
    setGameLost();
  }
}

static void onInput(int key, int action, int mods) {
  if (action != ACTION_PRESS && action != ACTION_REPEAT) {
    return;
  }

  if (key == KEY_ESCAPE) {
    cgaSetShouldClose(true);
    return;
  } else if (key == KEY_F3) {
    if (debugInfoEnabled) {
      debugInfoEnabled = false;
    } else {
      debugInfoEnabled = true;
    }

    return;
  }

  if (gameState == GS_INACTIVE) {
    return;
  }

  switch (key) {
    // Restart if lost
    case KEY_P:
    case KEY_SPACE:
      if (gameState == GS_LOST) {
        startGame();
      }
      return;

    // Reset game
    case KEY_R:
      startGame();
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

static int charCallback(char ch, int index, float* x, float* y) {
  return 1;
}

static void printDebugInfo() {
  if (debugBuffer == NULL) {
    debugBuffer = malloc(DEBUG_INFO_BUF_SIZE);

    if (debugBuffer == NULL) {
      printf("[ERROR] debugBuffer allocation failed :(");
      return;
    }
  }

  float fps = cgaGetFps();
  int frameCounter = cgaGetFrameCounter();
  int printedChars = sprintf_s(debugBuffer, DEBUG_INFO_BUF_SIZE, "FPS: %.0f", fps);

  if (printedChars > 0) {
    cgaSetCharDrawCallback(charCallback);
    glColor3f(0.0f, 1.0f, 0);
    cgaSetTextScale(0.5f, 0.5f);
    cgaDrawText(-1.0f, 1.0f, printedChars, debugBuffer);
    cgaSetCharDrawCallback(null);
  }
}

static void drawCellValue() {

}

static void setColor(int cellValue) {
  switch (cellValue) {
    case STARTING_VALUE:
      glColor3ub(238, 228, 218);
      break;

    case 4:
      glColor3ub(237, 224, 200);
      break;

    case 8:
      glColor3ub(242, 177, 121);
      break;

    case 16:
      glColor3ub(245, 149, 99);
      break;

    case 32:
      glColor3ub(246, 124, 95);
      break;

    case 64:
      glColor3ub(246, 94, 59);
      break;

    case 128:
      glColor3ub(237, 207, 114);
      break;

    case 256:
      glColor3ub(237, 204, 97);
      break;

    case 512:
      glColor3ub(237, 200, 80);
      break;

    case 1024:
      glColor3ub(237, 197, 63);
      break;

    case 2048:
      glColor3ub(237, 194, 46);
      break;
    
    default:
      glColor3ub(60, 58, 50);
      break;
  }
}

static void drawBoard(float ratio) {
  float startX = 0.5;
  float startY = 0.5 * ratio;
  float endX = -0.5;
  float endY = -0.5 * ratio;

  float cellSizeX = (endX - startX) / BOARD_WIDTH;
  float cellSizeY = (endY - startY) / BOARD_HEIGHT;

  float shrinkX = cellSizeX / 10.0f;
  float shrinkY = cellSizeY / 10.0f;

  const bufSize = 5;
  char scoreBuffer[bufSize];
  int wrote = 0;

  for (int x = 0; x < BOARD_WIDTH; x++) {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      int cellValue = getCell(x, y);

      if (cellValue != NO_CELL_VALUE) {
        setColor(cellValue);
      } else {
        glColor3f(0.5f, 0.5f, 0.5f);
      }

      float cStartX = startX + (x * cellSizeX) + shrinkX;
      float cStartY = startY + (y * cellSizeY) + shrinkY;
      float cEndX = cStartX + cellSizeX - shrinkX;
      float cEndY = cStartY + cellSizeY - shrinkY;

      drawQuad(cStartX, cStartY, cEndX, cEndY);

      if (cellValue != NO_CELL_VALUE) {
        glColor3f(1.0f, 1.0f, 1.0f);

        wrote = sprintf_s(scoreBuffer, bufSize, "%i", cellValue);

        if (wrote > 0) {
          float textShiftX = (wrote - 1) * (cellSizeX / 10.0f);
          float cSizeX = ((-cellSizeX / CH_BASE_X_SCALE) * 0.1) / ((float) wrote);
          float cSizeY = ((-cellSizeY / CH_BASE_Y_SCALE) * 0.1) / ((float) wrote);

          cgaSetTextScale(cSizeX, cSizeY);

          cgaDrawText(
            (cStartX + cellSizeX - (shrinkX * 2.5f) + textShiftX) /*/ ((float) wrote)*/,
            cStartY + shrinkY + textShiftX,
            bufSize,
            scoreBuffer
          );
        }
      }
    }
  }
}

void onUpdate(float deltaTime, float ratio) {
  gameTime += deltaTime;

  drawBoard(ratio);

  if (gameState == GS_LOST) {
    char* youLost = "You lose!";
    cgaSetTextScale(1, 1);

    float x = 0 - (5 * 8 * CH_BASE_X_SCALE);
    float y = 0.75f;

    glColor3f(0.0f, 0.5f, 0.0f);
    cgaDrawText(x + CH_BASE_X_SCALE, y - CH_BASE_Y_SCALE, 20, youLost);

    glColor3f(0, 1, 0);
    cgaDrawText(x, y, 20, youLost);
  }

  if (debugInfoEnabled) {
    printDebugInfo();
  }
}

void gameMain() {
  if (!cgaInit()) {
    return;
  }

  cgaSetKeyCallback(onInput);
  cgaSetFrameCallback(onUpdate);
  cgaInitTextDraw();
  cgaSetScreenTitle("2048");
  cgaSetScreenSize(800, 800);
  cgaSetVsync(true);

  startGame();

  cgaLoop();

  cgaClose();
  cgaCloseTextDraw();
}