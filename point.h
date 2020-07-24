#ifndef _H_point
#define _H_point

#include "raymath.h"

struct Point;
#include "box.h"
#include <initializer_list>

struct Point : Vector3 {
	static Point Zero();
	static Point North();
	static Point South();
	static Point East();
	static Point West();
	static Point Up();
	static Point Down();

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
	Point(float xx, float yy, float zz);

	Box box();
	float distance(Point p);
	Point round();
	Point tileCentroid();
	Point floor(float fy);
	float lineDistance(Point a, Point b);
	Point normalize();
	Point cross(Point);
	float dot(Point);
	Point scale(float);
	Point pivot(Point target, float speed);
	Point roundCardinal();
	Point rotateHorizontal();
	Point transform(Matrix);
	float length();
};

#endif