#include <math.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "tile_3d.h"
#include "engine.h"
#include "mesh.h"
#include "meshdata.h"

#include "render.h"
#include "vector.h"
#include "video.h"
#include "mathutil.h"


enum {
	TILE_RENDER_DOTS,
	TILE_RENDER_LINES,
	TILE_RENDER_QUADS,
	TILE_RENDER_MESH,
	TILE_RENDER_COUNT
};

typedef struct TilemapPos
{
	int x,y;
	int xs,ys;
} TilemapPos;

typedef struct TilemapRange
{
	int edgeX, edgeY;
	int x0,x1,y0,y1;
} TilemapRange;

typedef struct TileMeshInfo
{
	int numQuads;
	ScreenPoint **spStart;
} TileMeshInfo;


static uint8 tilemap3d[TILEMAP_SIZE];
static TilemapRange tilemapRange[TILEMAP_LAYERS];

static TilemapPos tmapPos[TILEMAP_LAYERS][TILEMAP_WIDTH];

static int tileRenderType = TILE_RENDER_DOTS;


#define NUM_TILES 4

static int8 *objTileMeshData[NUM_TILES] = { objRombusData, objCubeData, objGlenzData, objSquareCrossData };
static Mesh *objTileMesh[NUM_TILES];


static ScreenPoint tileScrPt[TILEMAP_SIZE];

static TileMeshInfo tileMeshInfo[TILEMAP_SIZE];
static ScreenPoint *scrPlist[TILEMAP_SIZE * 5];		// good theoritical maximum? Will reduce..
static int scrPindex = 0;

// scrPlist 4 ScreenPoint* per quad point at &tileScrPtr[n]
// tileMeshInfo per tile, tells you number of quads (can be 0 to 6 (but 5 max in our case as the bottom is always out of view)), then pointer to scrPlist start of the sequence of points

static ScreenPoint scrP0, scrP1, scrP2, scrP3;
static ScreenPoint *scrP[4] = { &scrP0, &scrP1, &scrP2, &scrP3 };


void advTileRenderType(bool inc)
{
	if (inc) {
		tileRenderType++;
		if (tileRenderType >= TILE_RENDER_COUNT) tileRenderType = 0;
	} else {
		tileRenderType--;
		if (tileRenderType < 0) tileRenderType = TILE_RENDER_COUNT - 1;
	}
}

void tilemap3dInit()
{
	memset(tilemap3d, 0, sizeof(tilemap3d));

	for (int i=0; i<TILEMAP_LAYERS; ++i) {
		uint8 n = (1 << (1+i)) - 1;
		uint8 *dst = &tilemap3d[i*TILEMAP_LAYER_SIZE];
		for (int y=0; y<TILEMAP_HEIGHT; ++y) {
			for (int x=0; x<TILEMAP_WIDTH; ++x) {
				uint8 c = 0;
				if (i==2) {
					if (!(x & n) || !(y & n)) c = 1;
				} else {
					if (!(x & n) && !(y & n)) c = 1;
				}
				*dst++ = c;
			}
		}
	}

	for (int i=0; i<NUM_TILES; ++i) {
		objTileMesh[i] = initMeshFromCPCdata(objTileMeshData[i]);
	}
}

static void drawDot(int xs, int ys, uint8 color, uint8 *vram)
{
	if (ys >= 0 && ys < SCR_H && xs >=0 && xs < SCR_W) {
		uint8 *dst = vram + VRAM_PIXEL_OFFSET((xs>>UNCHAINED_BITS),ys);
		*dst = color;
	}
}

static void drawRectangleLines(int x0, int y0, int x1, int y1, uint8 color, uint8 *vram)
{
	CLAMP(x0,0,SCR_W-1);
	CLAMP(x1,0,SCR_W-1);
	CLAMP(y0,0,SCR_H-1);
	CLAMP(y1,0,SCR_H-1);

	uint32 color32 = (color << 24) | (color << 16) | (color << 8) | color;

	if (x1 <= x0 || y1 <= y0) return;

	int countY = y1 - y0;
	for (int y = y0; y<=y1; y+=countY) {
		uint8 *dst = vram + VRAM_PIXEL_OFFSET((x0>>UNCHAINED_BITS),y);
		int16 length = x1 - x0;

		int16 xl = x0 & 3;
		if (xl) {
			int16 l = 4-xl;
			length -= l;
			while (l-- != 0) {
				*dst++ = color;
			};
		}

		uint32 *dst32 = (uint32*)dst;
		while(length > 3) {
			*dst32++ = color32;
			length-=4;
		};

		dst = (uint8*)dst32;
		while(length-- > 0) {
			*dst++ = color;
		};
	};

	uint8 *dstX0 = vram + VRAM_PIXEL_OFFSET((x0>>UNCHAINED_BITS),y0);
	uint8 *dstX1 = vram + VRAM_PIXEL_OFFSET((x1>>UNCHAINED_BITS),y0);
	for (int y = y0; y<y1; ++y) {
		*dstX0 = color;
		*dstX1 = color;
		dstX0 += SCR_LINE_BYTES;
		dstX1 += SCR_LINE_BYTES;
	}
}

static void drawRectangle(int x0, int y0, int x1, int y1, uint8 color, uint8 *vram)
{
	CLAMP(x0,0,SCR_W-1);
	CLAMP(x1,0,SCR_W-1);
	CLAMP(y0,0,SCR_H-1);
	CLAMP(y1,0,SCR_H-1);

	uint8 *dstY = vram + VRAM_PIXEL_OFFSET(0,y0);
	uint32 color32 = (color << 24) | (color << 16) | (color << 8) | color;

	int countY = y1 - y0;
	while (countY-- > 0) {
		uint8 *dst = dstY + (x0>>UNCHAINED_BITS);
		int16 length = x1 - x0;

		int16 xl = x0 & 3;
		if (xl) {
			int16 l = 4-xl;
			length -= l;
			while (l-- != 0) {
				*dst++ = color;
			};
		}

		uint32 *dst32 = (uint32*)dst;
		while(length > 3) {
			*dst32++ = color32;
			length-=4;
		};

		dst = (uint8*)dst32;
		while(length-- > 0) {
			*dst++ = color;
		};

		dstY += SCR_LINE_BYTES;
	};
}

static void cacheScreenData(Vec3 *pos, TilemapRange *tmapRange, uint8 layer, int layerZ)
{
	int x0 = tmapRange->x0;
	int x1 = tmapRange->x1;
	int y0 = tmapRange->y0;
	int y1 = tmapRange->y1;

	TilemapPos *tp = &tmapPos[layer][0];

	int px = -pos->x + x0 * TILE_SIZE;
	for (int x=x0; x<x1; ++x) {
		tp[x].x = px;
		tp[x].xs = (px << PROJ_BITS) / layerZ + SCR_W / 2;
		px += TILE_SIZE;
	}

	int py = -pos->y + y0 * TILE_SIZE;
	for (int y=y0; y<y1; ++y) {
		tp[y].y = py;
		tp[y].ys = (py << PROJ_BITS) / layerZ + SCR_H / 2;
		py += TILE_SIZE;
	}
}

static void findTilemapExtends(int posI, int iRange, int edgeI, int *tmapI0, int *tmapI1)
{
	int i0=0;
	int i1=iRange;

	bool isIn = false;
	for (int i=0; i<iRange; ++i) {
		if (posI > -edgeI && posI < edgeI) {
			if (!isIn) {
				i0 = i;
				isIn = true;
			}
		} else {
			if (isIn) {
				i1 = i;
				isIn = false;
			}
		}
		posI += TILE_SIZE;
	}

	if (i0 > 0) i0--;

	*tmapI0 = i0;
	*tmapI1 = i1;
}

static void updateTilemapEdges(Vec3 *pos, uint8 layer)
{
	TilemapRange *tmapRange = &tilemapRange[layer];
	int layerZ = pos->z + TILE_SIZE * (TILEMAP_LAYERS - layer);
	tmapRange->edgeX = (SCR_W/2 * layerZ) >> PROJ_BITS;
	tmapRange->edgeY = (SCR_H/2 * layerZ) >> PROJ_BITS;

	findTilemapExtends(-pos->x, TILEMAP_WIDTH, tmapRange->edgeX, &tmapRange->x0, &tmapRange->x1);
	findTilemapExtends(-pos->y, TILEMAP_HEIGHT, tmapRange->edgeY, &tmapRange->y0, &tmapRange->y1);

	cacheScreenData(pos, tmapRange, layer,layerZ);
}

/*static void renderTilemap3DLayerMeshOof(int x0, int y0, int x1, int y1, uint8 layer, int layerZ, uint8 *tmap, Screen *screen)
{
	// Once to update grid axes of the same and same cube, so it never renders just transforms cube object once
	renderMeshHack(objTileMesh[1], screen, true);

	TilemapPos *tp = &tmapPos[layer][0];

	for (int y=y0; y<y1; ++y) {
		const int py = tp[y].y;
		for (int x=x0; x<x1; ++x) {
			uint8 c = tmap[x];
			if (c > 0 && c < NUM_TILES) {
				const int px = tp[x].x;

				Mesh *ms = objTileMesh[c];
				ms->pos.x = px + 128;
				ms->pos.y = -py - 128;
				ms->pos.z = layerZ - 128;
				renderMeshHack(ms, screen, false);	// now only translate and project and render (transfrom grid axes happened already once for all)
			}
		}
		tmap += TILEMAP_WIDTH;
	}
}*/

static void renderTilemap3DLayerMesh(int x0, int y0, int x1, int y1, uint8 layer, int layerZ, uint8 *tmap, Screen *screen)
{
	TilemapPos *tp = &tmapPos[layer][0];
	ScreenPoint *sp = &tileScrPt[layer * TILEMAP_LAYER_SIZE + y0 * TILEMAP_WIDTH];

	int x1b = x1;
	int y1b = y1;
	if (x1 < TILEMAP_WIDTH-1) ++x1b;
	if (y1 < TILEMAP_HEIGHT-1) ++y1b;

	for (int y=y0; y<y1b; ++y) {
		const int py = tp[y].y;
		for (int x=x0; x<x1b; ++x) {
			const int px = tp[x].x;
			sp[x].x = px;
			sp[x].y = py;
		}
		sp += TILEMAP_WIDTH;
	}
}

static void renderTilemap3dLayerQuads(int x0, int y0, int x1, int y1, uint8 color, int tileScrSize, uint8 layer, uint8 *tmap, uint8 *vram)
{
	TilemapPos *tp = &tmapPos[layer][0];

	for (int y=y0; y<y1; ++y) {
		const int ys = tp[y].ys;
		for (int x=x0; x<x1; ++x) {
			if (tmap[x]) {
				const int xs = tp[x].xs;
				drawRectangle(xs,ys, xs+tileScrSize, ys+tileScrSize, color, vram);

				//scrP0.x = scrP3.x = xs << SCR_BITS;
				//scrP0.y = scrP1.y = (ys+tileScrSize) << SCR_BITS;
				//scrP1.x = scrP2.x = (xs+tileScrSize) << SCR_BITS;
				//scrP2.y = scrP3.y = ys << SCR_BITS;
				//drawQuad(scrP, color, vram);
			}
		}
		tmap += TILEMAP_WIDTH;
	}
}

static void renderTilemap3dLayerLines(int x0, int y0, int x1, int y1, uint8 color, int tileScrSize, uint8 layer, uint8 *tmap, uint8 *vram)
{
	TilemapPos *tp = &tmapPos[layer][0];

	for (int y=y0; y<y1; ++y) {
		const int ys = tp[y].ys;
		for (int x=x0; x<x1; ++x) {
			if (tmap[x]) {
				const int xs = tp[x].xs;
				drawRectangleLines(xs,ys, xs+tileScrSize, ys+tileScrSize, color, vram);
			}
		}
		tmap += TILEMAP_WIDTH;
	}
}

static void renderTilemap3dLayerDots(int x0, int y0, int x1, int y1, uint8 color, uint8 layer, uint8 *tmap, uint8 *vram)
{
	TilemapPos *tp = &tmapPos[layer][0];

	for (int y=y0; y<y1; ++y) {
		const int ys = tp[y].ys;
		for (int x=x0; x<x1; ++x) {
			if (tmap[x]) {
				drawDot(tp[x].xs, ys, color, vram);
			}
		}
		tmap += TILEMAP_WIDTH;
	}
}

void renderTilemap3dLayer(Vec3 *pos, uint8 layer, Screen *screen)
{
	updateTilemapEdges(pos, layer);

	TilemapRange *tmapRange = &tilemapRange[layer];
	const int x0 = tmapRange->x0;
	const int x1 = tmapRange->x1;
	const int y0 = tmapRange->y0;
	const int y1 = tmapRange->y1;

	uint8 color = ((layer+1) * 16) / TILEMAP_LAYERS;
	if (color > 15) color = 15;

	uint8 *tmap = &tilemap3d[layer*TILEMAP_LAYER_SIZE + y0*TILEMAP_WIDTH];
	uint8 *vram = (uint8*)screen->data;

	int layerZ, tileScrSize;
	if (tileRenderType > TILE_RENDER_DOTS) {
		layerZ = pos->z + TILE_SIZE * (TILEMAP_LAYERS - layer);
		tileScrSize = (TILE_SIZE << PROJ_BITS) / layerZ;
	}

	switch(tileRenderType) {
		case TILE_RENDER_DOTS:
			renderTilemap3dLayerDots(x0,y0,x1,y1,color,layer,tmap,vram);
		break;

		case TILE_RENDER_LINES:
			renderTilemap3dLayerLines(x0,y0,x1,y1,color,tileScrSize,layer,tmap,vram);
		break;

		case TILE_RENDER_QUADS:
			renderTilemap3dLayerQuads(x0,y0,x1,y1,color,tileScrSize,layer,tmap,vram);
		break;

		case TILE_RENDER_MESH:
			renderTilemap3DLayerMesh(x0,y0,x1,y1,layer,layerZ,tmap,screen);
		break;
	}
}

//262-1841 (131072)
//407-2121
//395-2096
