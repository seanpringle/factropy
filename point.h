#ifndef _H_point
#define _H_point

#include "raymath.h"

struct Point;
#include "box.h"
#include <initializer_list>

struct Point : Vector3 {
	Point();
	Point(std::initializer_list<float>);
	Point(Vector3);
	Point(float xx, float yy, float zz);

	Box box();
	float distance(Point p);
	Point floor(float fy);
};

#endif