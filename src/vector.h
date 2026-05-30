#ifndef VECTOR_H
#define VECTOR_H

#include "types.h"

typedef struct Vec3
{
	Vec3(int x, int y, int z) : x(x), y(y), z(z) {};
	Vec3() : x(0), y(0), z(0) {};
	
	inline Vec3 operator+(const Vec3& v) {
		return Vec3(x + v.x, y + v.y, z + v.z);
	}

	inline void operator+=(const Vec3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}

	inline Vec3 operator-(const Vec3& v) {
		return Vec3(x - v.x, y - v.y, z - v.z);
	}

	inline void operator-=(const Vec3& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
	}

	int x,y,z;
} Vec3;

#endif
