#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "game.h"
#include "cga_core.h"
#include "cga_window.h"
#include "cga_inputs.h"
#include "glutil.h"
#include "font_draw.h"
#include "log.h"
#include "cga_render.h"

#define BOARD_WIDTH 4
#define BOARD_HEIGHT 4
#define BOARD_SIZE (BOARD_HEIGHT * BOARD_WIDTH)
#define MOVE_TIME_SECS 0.5
#define DEBUG_INFO_BUF_SIZE 100
#define TARGET_VALUE 2048.0f
#define SCORE_BUF_LEN 20

#define TO_INDEX(x, y) ((BOARD_WIDTH * x) + y)
#define IN_BOUNDS(x, y) ((x >= 0 && x < BOARD_WIDTH) && (y >= 0 && y < BOARD_HEIGHT))
#define LERP(prog, a, b) (a + ((b - a) * prog))

#define NO_CELL_VALUE 0
#define STARTING_VALUE 2
#define OUT_OF_BOUNDS -1

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
static boolean debugInfoEnabled = true;

static char* debugBuffer = NULL;

static int freeSlots[BOARD_SIZE] = {0};
static int freeSlotCount = 0;

static int score = 0;
static int hiScore = 0;
static char scoreBuf[SCORE_BUF_LEN] = {0};

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

static boolean genRandomSlot();

static void startGame() {
  resetBoard();
  gameState = GS_ACTIVE;

  genRandomSlot();
  genRandomSlot();
}

static void setGameLost() {
  gameState = GS_LOST;
  logDebug("Game lost!");
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
      int nval = v + startVal;
      setCell(nextX, nextY, nval);
      score += nval;
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
  if (gameState != GS_ACTIVE) {
    return;
  }

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

static void printDebugInfo(float deltaTime) {
  if (debugBuffer == NULL) {
    debugBuffer = malloc(DEBUG_INFO_BUF_SIZE);

    if (debugBuffer == NULL) {
      printf("[ERROR] debugBuffer allocation failed :(");
      return;
    }
  }

  float fps = cgaGetFps();
  int frameCounter = cgaGetFrameCounter();
  int printedChars = sprintf_s(debugBuffer, DEBUG_INFO_BUF_SIZE, "FPS: %.0f\nDeltaTime: %fs", fps, deltaTime);

  float width = 0;
  float height = 0;
  
  cgaSetTextScale(0.5f, 0.5f);
  cgaMeasureText(DEBUG_INFO_BUF_SIZE, debugBuffer, &width, &height, null);

  float qX = -1.0f;
  float qY = 1.0f;

  glColor3f(0.0f, 0.75f, 0);
  drawQuad(qX, qY, qX + width, qY - height);

  if (printedChars > 0) {
    glColor3f(1.0f, 1.0f, 1.0f);
    cgaDrawText(-1.0f, 1.0f, printedChars, debugBuffer);
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

  const int bufSize = 5;
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
        if (cellValue < 8) {
          glColor3ub(119, 110, 101);
        } else {
          glColor3f(1.0f, 1.0f, 1.0f);
        }

        wrote = sprintf_s(scoreBuffer, bufSize, "%i", cellValue);

        if (wrote < 1) {
          continue;
        }

        float textShiftX = (wrote - 1) * (cellSizeX / 10.0f);
        float cSizeX = ((-cellSizeX / CH_BASE_X_SCALE) * 0.1) / ((float) wrote);
        float cSizeY = ((-cellSizeY / CH_BASE_Y_SCALE) * 0.1) / ((float) wrote);

        cgaSetTextScale(cSizeX, cSizeY);

        cgaDrawText(
          (cStartX + cellSizeX - (shrinkX * 2.5f) + textShiftX),
          cStartY + shrinkY + textShiftX,
          bufSize,
          scoreBuffer
        );
      }
    }
  }
}

static void drawScore(float ratio) {
  int len = sprintf_s(scoreBuf, SCORE_BUF_LEN, "Score: %i", score);

  if (len < 1) {
    logError("Error writing to score text buffer");
    return;
  }

  cgaSetTextScale(1, ratio);
  glColor3f(0.0f, 1.0f, 0.0f);

  cgaDrawText(-0.95f, -0.85f, SCORE_BUF_LEN, scoreBuf);
}

static void drawCenteredText(float y, int textLen, char* content) {
  cgaSetTextScale(1, 1);

  float width = 0;
  cgaMeasureText(textLen, content, &width, null, null);

  float x = 0 - (width / 2.0f);
  
  glColor3f(0.0f, 0.5f, 0.0f);
  cgaDrawText(x + CH_BASE_X_SCALE, y - CH_BASE_Y_SCALE, textLen, content);

  glColor3f(0, 1, 0);
  cgaDrawText(x, y, textLen, content);
}

void onUpdate(float deltaTime, float ratio) {
  gameTime += deltaTime;

  drawBoard(ratio);
  drawScore(ratio);

  if (gameState == GS_LOST) {
    char* youLost = "You lose!";
    char* restart = "'R' to restart";

    drawCenteredText(0.75f, 10, youLost);
    drawCenteredText(-0.65f, 20, restart);
  } else {
    drawCenteredText(0.75f, 6, "2048");
  }

  if (debugInfoEnabled) {
    printDebugInfo(deltaTime);
  }
}

static void bufferTest() {
  vertex_buffer buf = cgaGenVertexBuffer();

  if (buf == null) {
    logError("Failed to gen buffer");
    return;
  }

  float v1 = 1.32f;
  float v2 = 1.54f;
  float v3 = 3.5464f;
  int32_t i32 = 12343;

  cgaPushVertex2f(buf, v1, v2);
  cgaPushF32(buf, v3);
  cgaPushI32(buf, i32);

  logDebugF("buf.id=%i", buf->id);
  logDebugF("buf data: cap=%i len=%i", buf->capacity, buf->length);
  
  float* floatPtr = (float*) buf->data;

  logDebugF("float buf[0]: gotten=%f original=%f, equals=%i", floatPtr[0], v1, floatPtr[0] == v1);
  logDebugF("float buf[1]: gotten=%f original=%f, equals=%i", floatPtr[1], v2, floatPtr[1] == v2);
  logDebugF("float buf[2]: gotten=%f original=%f, equals=%i", floatPtr[2], v3, floatPtr[2] == v3);

  int32_t* intBuf = (int32_t*) (floatPtr);
  logDebugF("  int buf[3]: gotten=%8i original=%8i equals=%i", intBuf[3], i32, intBuf[3] == i32);

  cgaFreeVertexBuffer(buf);
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
  cgaSetVsync(false);

  bufferTest();

  startGame();

  cgaLoop();

  cgaClose();
  cgaCloseTextDraw();
}