#include "mathutil.h"

#include <math.h>

void initSinTab(const int numSines, const int repeats, const int amplitude, int *mySinTab)
{
	const int numSinesRepeat = numSines / repeats;
	float sMul = (2.0f * PI) / (float)(numSinesRepeat-1);

	for (int i=0; i<numSines; ++i) {
		*mySinTab++ = sin((float)i * sMul) * amplitude;
	}
}
