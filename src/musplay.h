#ifndef MUSPLAY_H
#define MUSPLAY_H

#ifdef SOUND_ON

#include "types.h"

enum {
	MUS_INTRO,
	MUS_GAME,
	MUS_NUM
};

bool loadMusDriver();
bool loadMusFile(int musIndex);
void playMusFile(int musIndex);
void stopMusPlay();
void shutdownMusPlay();

uint32 getMusTicks();

#endif

#endif