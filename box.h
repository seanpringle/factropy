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
	Box(Point a, Point b);
	Box(std::initializer_list<float>);

	Point centroid() const;
	BoundingBox bounds() const;
	Box translate(const Point p) const;
	Box translate(float x, float y, float z) const;
	Box grow(const Point p) const;
	Box grow(float n) const;
	Box grow(float x, float y, float z) const;
	Box shrink(const Point p) const;
	Box shrink(float n) const;
	Box shrink(float x, float y, float z) const;
	bool intersects(Box b) const;
	bool contains(Point p) const;
	float volume() const;
};

#endif