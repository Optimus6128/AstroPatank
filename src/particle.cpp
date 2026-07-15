#include "particle.h"

#include "mathutil.h"
#include "render.h"

typedef struct Particle
{
	Vec3 pos, vel;
	uint8 life, color;
} Particle;


static Particle particle[MAX_PARTICLES];
static uint32 currParticleIndex = 0;

void spawnParticle(Vec3 &pos, Vec3 &vel, uint8 color, uint8 life)
{
	Particle *p = &particle[currParticleIndex];

	p->pos = pos;
	p->vel = vel;
	p->color = color;
	p->life = life;

	currParticleIndex = (currParticleIndex + 1) % MAX_PARTICLES;
}

void updateParticles()
{
	for (int i=0; i< MAX_PARTICLES; ++i) {
		Particle *p = &particle[i];

		if (p->life != 0) {
			p->pos += p->vel;
			p->life--;
		}
	}
}

void renderParticles(int offsetFromPlayerX, int offsetFromPlayerY, int viewPosZ, uint8 *vram)
{
	for (int i=0; i<MAX_PARTICLES; ++i) {
		Particle *p = &particle[i];

		const uint8 life = p->life;
		if (life != 0) {
			Vec3 *pos = &p->pos;
			int sx = ((SCR_W/2) << SCR_BITS) + (((offsetFromPlayerX + pos->x) << (SCR_BITS + PROJ_BITS - PPOS_BITS)) / viewPosZ);
			int sy = ((SCR_H/2) << SCR_BITS) + (((offsetFromPlayerY + pos->y) << (SCR_BITS + PROJ_BITS - PPOS_BITS)) / viewPosZ);

			if (sx >= 0 && sx < ((SCR_W-1) << SCR_BITS) && sy >= 0 && sy < ((SCR_H-1) << SCR_BITS)) {
				int alphaShade = 8 * life;
				if (alphaShade > SHADE_ALPHA_MAX) alphaShade = SHADE_ALPHA_MAX;
				renderAntialiasedDot(sx, sy, p->color, alphaShade, vram);
			}
		}
	}
}
