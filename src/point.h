#pragma once

// Extend Raylib's Vector3 to C++ land.

struct Point;

#include "raylib-ex.h"
#include "box.h"
#include "sphere.h"
#include "mat4.h"
#include "volume.h"
#include <initializer_list>

struct Point : Vector3 {
	static const Point Zero;
	static const Point North;
	static const Point South;
	static const Point East;
	static const Point West;
	static const Point Up;
	static const Point Down;

	bool operator==(const Point& o) const;
	bool operator!=(const Point& o) const;
	bool operator<(const Point& o) const;
	Point operator+(const Point& o) const;
	Point operator+(float n) const;
	Point operator-(const Point& o) const;
	Point operator-(float n) const;
	Point operator-() const;
	Point operator*(const Point& o) const;
	Point operator*(float s) const;
	void operator+=(const Point& o);
	void operator-=(const Point& o);

	operator const std::string() const {
		return fmt("{x:%0.2f,y:%0.2f,z:%0.2f}", x, y, z);
	};

	Point();
	Point(std::initializer_list<float>);
	Point(Vector3);
	Point(Volume);
	Point(float xx, float yy, float zz);

	Box box() const;
	Sphere sphere() const;
	float distanceSquared(Point p) const;
	float distance(Point p) const;
	Point round() const;
	Point tileCentroid() const;
	Point floor(float fy) const;
	float lineDistance(Point a, Point b) const;
	Point normalize() const;
	Point cross(Point) const;
	float dot(Point) const;
	Point scale(float) const;
	Point pivot(Point target, float speed) const;
	Point roundCardinal() const;
	Point oppositeCardinal() const;
	Point rotateHorizontal() const;
	Point randomHorizontal() const;
	bool cardinalParallel(Point) const;
	Point transform(Mat4) const;
	float length() const;
	Mat4 rotation();
	Mat4 translation();
	Point nearestPointOnLine(Point l0, Point l1) const;
	static bool linesCrossOnGround(Point a0, Point a1, Point b0, Point b1);
};

namespace std	{
	template<> struct hash<Point> {
		std::size_t operator()(Point const& p) const noexcept {
			return std::hash<float>{}(p.distanceSquared(Point::Zero));
		}
	};
}
