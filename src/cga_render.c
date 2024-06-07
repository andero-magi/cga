#include "cga_render.h"
#include "cga_core.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_BUFFER_SIZE 1024
#define VEC2_SIZE (sizeof(float) * 2)
#define OFFSET_PTR(buf) (buf->data + buf->length)

vertex_buffer cgaGenVertexBuffer() {
  uint32_t id = 0;
  glGenVertexArrays(1, &id);

  if (id == 0) {
    return null;
  }

  int memSize = sizeof(vertex_buffer_t);
  vertex_buffer ptr = malloc(memSize);

  if (ptr == null) {
    logError("Failed to allocate vertex buffer struct");
    return null;
  }

  uint8_t* databuf = malloc(INITIAL_BUFFER_SIZE);

  ptr->length = 0;
  ptr->id = id;

  if (databuf == null) {
    logError("Failed to allocate array for vertex buffer data");
    ptr->capacity = 0;
    ptr->data = null; 
  } else {
    ptr->capacity = INITIAL_BUFFER_SIZE;
    ptr->data = databuf;
  }

  return ptr;
}

void cgaFreeVertexBuffer(vertex_buffer buf) {
  if (buf == null) {
    return;
  }

  if (buf->id != null) {
    glDeleteVertexArrays(1, &buf->id);
  }

  if (buf->data != null) {
    free(buf->data);
    buf->data = null;
  }

  free(buf);
}

void cgaClearBuffer(vertex_buffer buf) {
  if (buf == null || buf->capacity < 1) {
    return;
  }

  buf->length = 0;
}

void cgaBindBuffer(vertex_buffer buf) {
  if (buf == null) {
    glBindVertexArray(null);
    return;
  }

  glBindVertexArray(buf->id);
}

void cgaUploadBuffer(vertex_buffer buf, GLenum usage) {
  glBufferData(GL_ARRAY_BUFFER, buf->length, buf->data, usage);
}

static boolean ensureWritable(vertex_buffer buf, int bytesToWrite) {
  uint32_t cap = buf->capacity;
  uint32_t len = buf->length;
  uint32_t nlen = bytesToWrite + len;

  if (nlen <= cap) {
    return true;
  }

  uint32_t ncap = ((nlen / INITIAL_BUFFER_SIZE) + 1) * INITIAL_BUFFER_SIZE;
  uint8_t* nptr = realloc(buf->data, ncap);

  if (nptr == null) {
    logError("Failed to expand vertex array");
    return false;
  }

  buf->data = nptr;
  buf->capacity = ncap;

  return true;
}

void cgaPushVertex2f(vertex_buffer buf, float x, float y) {
  if (!ensureWritable(buf, VEC2_SIZE)) {
    return;
  }

  float* fPtr = (float*) OFFSET_PTR(buf);
  fPtr[0] = x;
  fPtr[1] = y;

  buf->length += VEC2_SIZE;
}

#define PUSH_SINGLE_VALUE(type, val, buf) \
  if (!ensureWritable(buf, sizeof(type))) {\
    return;\
  }\
  type* ptr = (type*) (buf->data + buf->length);\
  ptr[0] = val;\
  buf->length += sizeof(type);

void cgaPushF32(vertex_buffer buf, float f) {
  PUSH_SINGLE_VALUE(float, f, buf)
}
void cgaPushF64(vertex_buffer buf, double d)
 {
  PUSH_SINGLE_VALUE(double, d, buf)
}

void cgaPushI32(vertex_buffer buf, int32_t i) {
  PUSH_SINGLE_VALUE(int32_t, i, buf)
}

void cgaPushU32(vertex_buffer buf, uint32_t i) {
  PUSH_SINGLE_VALUE(uint32_t, i, buf)
}

void cgaPushU8(vertex_buffer buf, uint8_t u8) {
  PUSH_SINGLE_VALUE(uint8_t, u8, buf)
}
