#include <math.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "intro.h"

#include "input.h"
#include "sound.h"
#include "musplay.h"
#include "mathutil.h"
#include "vector.h"
#include "engine.h"
#include "render.h"
#include "mesh.h"

#include "meshdata.h"

enum {
	OBJ_UFO, OBJ_UFO2,
	OBJ_CUBESTAR, OBJ_ROMBUS_RING, 
	OBJ_LETTER_A,
	OBJ_LETTER_S,
	OBJ_LETTER_T,
	OBJ_LETTER_R,
	OBJ_LETTER_O,
	OBJ_LETTER_P,
	OBJ_LETTER_N,
	OBJ_LETTER_K,
	NUM_MESHES
};

static int8 *objMeshData[NUM_MESHES] =	{ 	objUfoData, objUfo2Data, objCubeStarData, objRombusRingData, 
											objLetterAData, objLetterSData, objLetterTData, objLetterRData, objLetterOData, objLetterPData, objLetterNData, objLetterKData
										};

static Mesh *objMesh[NUM_MESHES];

static void inputMenu()
{
	static bool upPressed = false;
	static bool downPressed = false;
	static bool leftPressed = false;
	static bool rightPressed = false;

	if (buttonsHeld.up & !upPressed) {
		setIsInGame(true);
	}
	if (buttonsHeld.down & !downPressed) {
	}	
	if (buttonsHeld.left & !leftPressed) {
	}
	if (buttonsHeld.right & !rightPressed) {
	}	

	upPressed = buttonsHeld.up;
	downPressed = buttonsHeld.down;
	leftPressed = buttonsHeld.left;
	rightPressed = buttonsHeld.right;
}

static void updateMenu(Screen *screen, int t)
{
	Mesh *ms = objMesh[OBJ_CUBESTAR];

	ms->rot.x = t;
	ms->rot.y = 2*t;
	ms->rot.z = 3*t;

	ms->pos.x = 0;
	ms->pos.y = 0;
	ms->pos.z = 4096;

	renderMesh(ms, screen);
}

void menuInit()
{
	for (int i=0; i<NUM_MESHES; ++i) {
		objMesh[i] = initMeshFromCPCdata(objMeshData[i]);
	}

	//runMusPlayTest();
}

void menuRun(Screen *screen, int t)
{
	inputMenu();
	updateMenu(screen, t);
}
