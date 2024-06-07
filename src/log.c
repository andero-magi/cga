#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "log.h"
#include "cga_core.h"

#define BUF_SIZE 250
#define TIME_BUF 100

typedef struct {
  char name[6];
} level_info_t;

static char msgBuf[BUF_SIZE] = {0};
static char timeBuffer[TIME_BUF] = {0};

static level_info_t levelInfoTable[4] = {
  {.name = " INFO"},
  {.name = "DEBUG"},
  {.name = " WARN"},
  {.name = "ERROR"}
};

static boolean formatTime() {
  time_t now = time(null);
  int result = strftime(timeBuffer, TIME_BUF, "%H:%M:%S", localtime(&now));
  return result > 0;
}

void cgaLog(log_level_t level, char* message, ...) {
  va_list args;
  va_start(args, message);
  
  int printed = vsnprintf(msgBuf, BUF_SIZE, message, args);
  va_end(args);

  if (printed < 1) {
    return;
  }

  void* outstream;

  if (level == LL_ERROR) {
    outstream = stderr;
  } else {
    outstream = stdout;
  }

  level_info_t inf = levelInfoTable[level];

  if (formatTime()) {
    fprintf(outstream, "[%s %s] %s\n", timeBuffer, inf.name, msgBuf);
  } else {
    fprintf(outstream, "[%s] %s\n", inf.name, msgBuf);
  }
}