#include "common.h"
#include "mat4.h"

const Mat4 Mat4::identity = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};

Mat4::Mat4() {
	m0  = 0;
	m4  = 0;
	m8  = 0;
	m12 = 0;
	m1  = 0;
	m5  = 0;
	m9  = 0;
	m13 = 0;
	m2  = 0;
	m6  = 0;
	m10 = 0;
	m14 = 0;
	m3  = 0;
	m7  = 0;
	m11 = 0;
	m15 = 0;
}

Mat4::Mat4(const Matrix& o) {
	m0  = o.m0;
	m4  = o.m4;
	m8  = o.m8;
	m12 = o.m12;
	m1  = o.m1;
	m5  = o.m5;
	m9  = o.m9;
	m13 = o.m13;
	m2  = o.m2;
	m6  = o.m6;
	m10 = o.m10;
	m14 = o.m14;
	m3  = o.m3;
	m7  = o.m7;
	m11 = o.m11;
	m15 = o.m15;
}

Mat4::Mat4(std::initializer_list<float> l) {
	auto i = l.begin();
	m0  = *i++;
	m4  = *i++;
	m8  = *i++;
	m12 = *i++;
	m1  = *i++;
	m5  = *i++;
	m9  = *i++;
	m13 = *i++;
	m2  = *i++;
	m6  = *i++;
	m10 = *i++;
	m14 = *i++;
	m3  = *i++;
	m7  = *i++;
	m11 = *i++;
	m15 = *i++;
}

Mat4 Mat4::scale(float x, float y, float z) {
	return MatrixScale(x, y, z);
}

Mat4 Mat4::scale(float s) {
	return MatrixScale(s, s, s);
}

Mat4 Mat4::rotate(Vector3 axis, float radians) {
	return MatrixRotate(axis, radians);
}

Mat4 Mat4::rotateX(float radians) {
	return MatrixRotateX(radians);
}

Mat4 Mat4::rotateY(float radians) {
	return MatrixRotateY(radians);
}

Mat4 Mat4::rotateZ(float radians) {
	return MatrixRotateZ(radians);
}

Mat4 Mat4::translate(float x, float y, float z) {
	return MatrixTranslate(x, y, z);
}

Mat4 Mat4::translate(Point p) {
	return translate(p.x, p.y, p.z);
}

float Mat4::determinant() const {
	return
		(m12 * m9 * m6  * m3 ) - (m8 * m13 * m6  * m3 ) - (m12 * m5 * m10 * m3 ) + (m4 * m13 * m10 * m3 ) +
		(m8  * m5 * m14 * m3 ) - (m4 * m9  * m14 * m3 ) - (m12 * m9 * m2  * m7 ) + (m8 * m13 * m2  * m7 ) +
		(m12 * m1 * m10 * m7 ) - (m0 * m13 * m10 * m7 ) - (m8  * m1 * m14 * m7 ) + (m0 * m9  * m14 * m7 ) +
		(m12 * m5 * m2  * m11) - (m4 * m13 * m2  * m11) - (m12 * m1 * m6  * m11) + (m0 * m13 * m6  * m11) +
		(m4  * m1 * m14 * m11) - (m0 * m5  * m14 * m11) - (m8  * m5 * m2  * m15) + (m4 * m9  * m2  * m15) +
		(m8  * m1 * m6  * m15) - (m0 * m9  * m6  * m15) - (m4  * m1 * m10 * m15) + (m0 * m5  * m10 * m15);
}

Mat4 Mat4::normalize() const {
	float det = determinant();
	Matrix result;
	result.m0  = m0/det;
	result.m1  = m1/det;
	result.m2  = m2/det;
	result.m3  = m3/det;
	result.m4  = m4/det;
	result.m5  = m5/det;
	result.m6  = m6/det;
	result.m7  = m7/det;
	result.m8  = m8/det;
	result.m9  = m9/det;
	result.m10 = m10/det;
	result.m11 = m11/det;
	result.m12 = m12/det;
	result.m13 = m13/det;
	result.m14 = m14/det;
	result.m15 = m15/det;
	return result;
}

Mat4 Mat4::operator*(const Mat4& o) const {
	Matrix result;
	result.m0  = m0*o.m0  + m1*o.m4  + m2*o.m8   + m3*o.m12;
	result.m1  = m0*o.m1  + m1*o.m5  + m2*o.m9   + m3*o.m13;
	result.m2  = m0*o.m2  + m1*o.m6  + m2*o.m10  + m3*o.m14;
	result.m3  = m0*o.m3  + m1*o.m7  + m2*o.m11  + m3*o.m15;
	result.m4  = m4*o.m0  + m5*o.m4  + m6*o.m8   + m7*o.m12;
	result.m5  = m4*o.m1  + m5*o.m5  + m6*o.m9   + m7*o.m13;
	result.m6  = m4*o.m2  + m5*o.m6  + m6*o.m10  + m7*o.m14;
	result.m7  = m4*o.m3  + m5*o.m7  + m6*o.m11  + m7*o.m15;
	result.m8  = m8*o.m0  + m9*o.m4  + m10*o.m8  + m11*o.m12;
	result.m9  = m8*o.m1  + m9*o.m5  + m10*o.m9  + m11*o.m13;
	result.m10 = m8*o.m2  + m9*o.m6  + m10*o.m10 + m11*o.m14;
	result.m11 = m8*o.m3  + m9*o.m7  + m10*o.m11 + m11*o.m15;
	result.m12 = m12*o.m0 + m13*o.m4 + m14*o.m8  + m15*o.m12;
	result.m13 = m12*o.m1 + m13*o.m5 + m14*o.m9  + m15*o.m13;
	result.m14 = m12*o.m2 + m13*o.m6 + m14*o.m10 + m15*o.m14;
	result.m15 = m12*o.m3 + m13*o.m7 + m14*o.m11 + m15*o.m15;
	return result;
}
