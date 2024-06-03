#ifndef FONT_DRAW_H
#define FONT_DRAW_H

typedef void(*char_callback_t)(char ch, float* x, float* y);

void cgaSetCallback(char_callback_t callback);

void cgaDrawText(float x, float y, const char* content);

#endif // FONT_DRAW_H