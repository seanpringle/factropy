#ifndef _H_mat4
#define _H_mat4

struct Mat4;

#include "raymath.h"
#include "point.h"
#include <initializer_list>

struct Mat4 : Matrix {
	static const Mat4 identity;

	Mat4 operator*(const Mat4& o) const;

	Mat4();
	Mat4(std::initializer_list<float>);
	Mat4(const Matrix& o);
	float determinant() const;
	Mat4 normalize() const;

	static Mat4 scale(float x, float y, float z);
	static Mat4 rotate(Vector3 axis, float radians);
	static Mat4 rotateX(float radians);
	static Mat4 rotateY(float radians);
	static Mat4 rotateZ(float radians);
	static Mat4 translate(float x, float y, float z);
	static Mat4 translate(Point p);
};

#endif