#ifndef DEMO_H
#define DEMO_H

#include "types.h"

#define SCR_W 320
#define SCR_H 200
#define SCR_BPP 8

#define SCR_UNCHAINED

#ifndef SCR_UNCHAINED
	#define SCR_LINE_BYTES ((SCR_W * SCR_BPP) / 8)
	#define VRAM_PIXEL_OFFSET(x,y) ((y) * SCR_LINE_BYTES + (x))
#else
	#define SCR_LINE_BYTES (((SCR_W / 4) * SCR_BPP) / 8)
	#define VRAM_PIXEL_OFFSET(x,y) ((y) * SCR_LINE_BYTES + ((x >> 2)))
#endif


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
