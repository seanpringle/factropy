#ifndef _H_point
#define _H_point

#include "raymath.h"

struct Point;
#include "box.h"

struct Point {
	float x, y, z;

	Vector3 vec();
	Box box();
	float distance(Point p);
};

#endif