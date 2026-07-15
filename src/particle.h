#ifndef PARTICLE_H
#define PARTICLE_H

#include "types.h"
#include "vector.h"

#define MAX_PARTICLES 256

void initParticles();

void spawnParticle(Vec3& pos, Vec3& vel, uint8 color, uint8 life);
void updateParticles();
void renderParticles(int offsetFromPlayerX, int offsetFromPlayerY, int viewPosZ, uint8 *vram);

#endif
