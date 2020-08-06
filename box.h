#ifndef _H_box
#define _H_box

#include "raymath.h"

struct Box;
#include "point.h"
#include "volume.h"
#include <initializer_list>

struct Box {
	float x, y, z, w, h, d;

	Box();
	Box(Point p, Volume v);
	Box(std::initializer_list<float>);

	Point centroid();
	BoundingBox bounds();
	Box translate(const Point p);
	Box translate(float x, float y, float z);
	Box grow(const Point p);
	Box grow(float n);
	Box grow(float x, float y, float z);
	Box shrink(const Point p);
	Box shrink(float n);
	Box shrink(float x, float y, float z);
	bool intersects(Box b);
	bool contains(Point p);
};

#endif