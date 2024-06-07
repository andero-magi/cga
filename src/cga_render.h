#ifndef CGA_RENDER_H
#define CGA_RENDER_H

#include <GL/glew.h>
#include <stdint.h>

typedef struct VertexBuffer {
  uint32_t id;
  uint32_t capacity;
  uint32_t length;
  uint8_t* data;
} vertex_buffer_t;

typedef vertex_buffer_t* vertex_buffer;

vertex_buffer cgaGenVertexBuffer();

void cgaFreeVertexBuffer(vertex_buffer buf);

void cgaBindBuffer(vertex_buffer buf);

void cgaUploadBuffer(vertex_buffer buf, GLenum usage);

void cgaClearBuffer(vertex_buffer buf);

void cgaPushVertex2f(vertex_buffer buf, float x, float y);

void cgaPushF32(vertex_buffer buf, float f);

void cgaPushF64(vertex_buffer buf, double d);

void cgaPushI32(vertex_buffer buf, int32_t i);

void cgaPushU32(vertex_buffer buf, uint32_t i);

void cgaPushU8(vertex_buffer buf, uint8_t u8);

#endif // CGA_RENDER_H