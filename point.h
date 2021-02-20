#ifndef _H_point
#define _H_point

#include "raymath.h"

struct Point;
#include "box.h"
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

	Point();
	Point(std::initializer_list<float>);
	Point(Vector3);
	Point(Volume);
	Point(float xx, float yy, float zz);

	Box box() const;
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
	Point rotateHorizontal() const;
	Point randomHorizontal() const;
	Point transform(Mat4) const;
	float length() const;
	Mat4 rotation();
	Mat4 translation();
	Point nearestPointOnLine(Point l0, Point l1) const;
	static bool linesCrossOnGround(Point a0, Point a1, Point b0, Point b1);
};

#endif