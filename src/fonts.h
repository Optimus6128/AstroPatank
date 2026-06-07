#ifndef FONTS_H
#define FONTS_H

#include "types.h"

void fontsInit();
void drawText(int xp, int yp, char *text, uint8 colOffset, int scaleBits, uint8 *vram);

#endif
