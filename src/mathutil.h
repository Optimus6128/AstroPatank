#ifndef MATHUTIL_H
#define MATHUTIL_H

#include "types.h"

#ifndef PI
	#define PI 3.14159265359f
#endif

#define CLAMP(a,min,max) { if (a < min) a = min; if (a > max) a = max; }
#define CLAMP_LEFT(a,min) { if (a < min) a = min; }
#define CLAMP_RIGHT(a,max) { if (a > max) a = max; }

void initSinTab(const int numSines, const int repeats, const int amplitude, int *mySinTab);
int getRand(int from, int to);

#endif
