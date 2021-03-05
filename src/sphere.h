#pragma once

struct Sphere;
#include "raylib-ex.h"
#include "point.h"
#include <initializer_list>

struct Sphere {
	float x, y, z, r;

	Sphere();
	Sphere(Point p, float radius);
	Sphere(float x, float y, float z, float radius);
	Sphere(std::initializer_list<float>);

	operator const std::string() const {
		return fmt("{x:%0.2f,y:%0.2f,z:%0.2f,r:%0.2f}", x, y, z, r);
	};

	Point centroid() const;
	Sphere grow(float n) const;
	Sphere shrink(float n) const;
	Sphere translate(const Point p) const;
	Sphere translate(float x, float y, float z) const;
	bool intersects(Sphere b) const;
	bool contains(Point p) const;

};
