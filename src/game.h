#ifndef GAME_H
#define GAME_H

#include "types.h"
#include "mesh.h"
#include "vector.h"

#ifndef __DJGPP__
	#define SOUND_ON
#endif

#define SCR_W 320
#define SCR_H 200
#define SCR_BPP 8

//#define SCR_UNCHAINED

#ifndef SCR_UNCHAINED
	#define UNCHAINED_BITS 0
	#define SCR_LINE_BYTES ((SCR_W * SCR_BPP) / 8)
#else
	#define UNCHAINED_BITS 2
	#define SCR_LINE_BYTES (((SCR_W >> UNCHAINED_BITS) * SCR_BPP) / 8)
#endif

#define VRAM_PIXEL_OFFSET(x,y) ((y) * SCR_LINE_BYTES + (x))

#define PPOS_BITS 8

typedef struct Screen
{
	int width, height, bpp;
	void *data;
}Screen;

typedef struct GameThing
{
	Vec3 pos, rot, vel;
	int size;
	Mesh *mesh;
	int spawn;
	Vec3 spawnMeshScale;
	bool alive;
}GameThing;


void gameInit();
void gameRun(Screen *screen, int t);

void setIsInGame(bool inGame);
bool getIsInGame();
void setGameQuit(bool quit);
bool isGameQuit();
void initNewGameStart(int difficulty);

void startGameMusic(int musIndex);

void playerFire(Vec3 &pos, int angle);
bool checkThingMapCollision(GameThing *gt);
void spawnParticle(Vec3 &pos, Vec3 &vel, uint8 color, uint8 life);
Vec3 getVelocityFromAngle(int angle, int scale);

#endif
