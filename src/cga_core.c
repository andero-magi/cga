#include "cga_core.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* cgaFormatString(int maxlen, char* format, ...) {
  char buf[maxlen];

  va_list args;
  va_start(args, format);

  int length = vsnprintf(buf, maxlen, format, args);

  va_end(args);

  if (length < 1) {
    return null;
  }

  int memoryLength = (length + 1) * sizeof(char);
  char* resultBuf = malloc(memoryLength);

  memcpy_s(resultBuf, memoryLength, buf, memoryLength);

  return resultBuf;
}
