#include "glutil.h"

void drawQuad(float startX, float startY, float endX, float endY) {
  glBegin(GL_QUADS);
    glVertex2f(startX, startY); // Bottom-left
    glVertex2f(endX, startY);   // Bottom-right
    glVertex2f(endX, endY);     // Top-right
    glVertex2f(startX, endY);   // Top-left
  glEnd();
}