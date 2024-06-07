#ifndef FONT_DRAW_H
#define FONT_DRAW_H

#define CHAR_DIF_X 8.5f
#define CHAR_DIF_Y 8.5f

#define CH_BASE_X_SCALE 0.015
#define CH_BASE_Y_SCALE 0.015

typedef int(*char_callback_t)(char ch, int chIndex, float* x, float* y);

int cgaTextDrawIsInitialized();

boolean cgaInitTextDraw();

void cgaCloseTextDraw();

void cgaSetTextScale(float xScale, float yScale);

void cgaSetCharDrawCallback(char_callback_t callback);

void cgaDrawText(float x, float y, int maxbufSize, const char* content);

void cgaMeasureText(int maxBufSize, const char* content, float* width, float* height, int* lines);

#endif // FONT_DRAW_H