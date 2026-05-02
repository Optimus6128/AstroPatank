#ifndef DEMO_H
#define DEMO_H

#include "types.h"

#define SCR_W 320
#define SCR_H 200
#define SCR_BPP 8
#define SCR_BYTE_LENGTH ((SCR_W * SCR_BPP) / 8)
#define VRAM_PIXEL_OFFSET(x,y) ((y) * SCR_BYTE_LENGTH + (x))

typedef struct Rectangle
{
	Rectangle(int x0, int y0, int x1, int y1) : x0(x0), y0(y0), x1(x1), y1(y1) {};
	Rectangle() : x0(0), y0(0), x1(0), y1(0) {};

	int x0,y0;
	int x1,y1;
} Rectangle;

typedef struct Screen
{
	int width, height, bpp;
	void *data;
} Screen;

enum {
	FX_3D,
	FX_NUM
};

void fx3dInit(bool onlySetup);
void fx3dRun(Screen *screen, int t);

void demoInit(int partSelect);
void demoRun(Screen *screen, int t);

#endif
