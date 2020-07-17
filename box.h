#ifndef _H_box
#define _H_box

#include "raymath.h"

struct Box;
#include "point.h"

struct Box {
	float x, y, z, w, h, d;

	Point centroid();
	BoundingBox bounds();
	Box translate(Point p);
	Box grow(Point p);
	Box shrink(Point p);
	bool intersects(Box b);
};

#endif