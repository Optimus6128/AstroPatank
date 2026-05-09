#include <math.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "tile_3d.h"
#include "engine.h"
#include "video.h"

typedef struct TilemapRange
{
	int edgeX;
	int edgeY;
} TilemapRange;


static uint8 tilemap3d[TILEMAP_SIZE];
static TilemapRange tilemapRange[TILEMAP_LAYERS];

void tilemap3dInit()
{
	memset(tilemap3d, 0, sizeof(tilemap3d));

	for (int i=0; i<TILEMAP_LAYERS; ++i) {
		uint8 n = (1 << (1+i)) - 1;
		uint8 *dst = &tilemap3d[i*TILEMAP_LAYER_SIZE];
		for (int y=0; y<TILEMAP_HEIGHT; ++y) {
			for (int x=0; x<TILEMAP_WIDTH; ++x) {
				uint8 c = 0;
				if (!(x & n) && !(y & n)) c = 1;
				*dst++ = c;
			}
		}
	}
}

static void drawDot(int xs, int ys, uint8 color, uint8 *vram)
{
	if (ys >= 0 && ys < SCR_H && xs >=0 && xs < SCR_W) {
		uint8 *dst = vram + VRAM_PIXEL_OFFSET((xs>>UNCHAINED_BITS),ys);
		*dst = color;
	}
}


static int xsData[TILEMAP_WIDTH];
static int x0,x1;

static void cacheScreenXdata(Vec3 *pos, int edgeX, int edgeY, int pz)
{
	x0=0;
	x1=TILEMAP_WIDTH;

	bool xIsIn = false;
	int px = -pos->x;
	for (int x=0; x<TILEMAP_WIDTH; ++x) {
		if (px > -edgeX && px < edgeX) {
			int xs = (px << PROJ_BITS) / pz + SCR_W / 2;
			xsData[x] = xs;
			if (!xIsIn) {
				x0 = x;
				xIsIn = true;
			}
		} else {
			if (xIsIn) {
				x1 = x;
				xIsIn = false;
			}
		}
		px += TILE_SIZE;
	}
}

void renderTilemap3dLayer(Vec3 *pos, uint8 layer, Screen *screen)
{
	uint8 *tmap = &tilemap3d[layer * TILEMAP_LAYER_SIZE];
	uint8 *vram = (uint8*)screen->data;

	const int edgeX = tilemapRange[layer].edgeX;
	const int edgeY = tilemapRange[layer].edgeY;

	int pz = pos->z + TILE_SIZE * (TILEMAP_LAYERS - layer);

	cacheScreenXdata(pos, edgeX, edgeY, pz);

	uint8 color = ((layer+1) * 16) / TILEMAP_LAYERS;
	if (color > 15) color = 15;

	int py = -pos->y;
	uint8 *src = &tilemap3d[layer*TILEMAP_LAYER_SIZE];
	for (int y=0; y<TILEMAP_HEIGHT; ++y) {
		if (py > -edgeY && py < edgeY) {
			int ys = (py << PROJ_BITS) / pz + SCR_H / 2;
			for (int x=x0; x<x1; ++x) {
				if (src[x]) {
					drawDot(xsData[x],ys, color, vram);
				}
			}
		}
		src += TILEMAP_WIDTH;
		py += TILE_SIZE;
	}
}

static void updateTilemapEdges(Vec3 *pos)
{
	for (int i=0; i<TILEMAP_LAYERS; ++i) {
		TilemapRange *tmapRange = &tilemapRange[i];
		int pz = pos->z + TILE_SIZE * (TILEMAP_LAYERS - i);
		tmapRange->edgeX = ((SCR_W/2) * pz) >> PROJ_BITS;
		tmapRange->edgeY = ((SCR_H/2) * pz) >> PROJ_BITS;
	}
}

//262-1841 (131072)
//407-2121
/*static void printSomething()
{
	for (int i=0; i<TILEMAP_LAYERS; ++i) {
		printf("%d,%d   ", tilemapRange[i].edgeX, tilemapRange[i].edgeY);
	}
	printf("\n");
}*/

void renderTilemap3d(Vec3 *pos, Screen *screen)
{
	updateTilemapEdges(pos);

	for (int i=0; i<TILEMAP_LAYERS; ++i) {
		renderTilemap3dLayer(pos, i, screen);
	}

	//printSomething();
}
