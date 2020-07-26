#include "common.h"
#include "box.h"
#include "sim.h"

Point Point::Zero() {
	return Point( 0, 0, 0);
}

Point Point::North() {
	return Point( 0, 0,-1);
}

Point Point::South() {
	return Point( 0, 0, 1);
}

Point Point::East() {
	return Point( 1, 0, 0);
}

Point Point::West() {
	return Point(-1, 0, 0);
}

Point Point::Up() {
	return Point( 0, 1, 0);
}

Point Point::Down() {
	return Point( 0,-1, 0);
}

Point::Point() {
	x = 0;
	y = 0;
	z = 0;
}

Point::Point(std::initializer_list<float> l) {
	auto i = l.begin();
	x = *i++;
	y = *i++;
	z = *i++;
}

Point::Point(float xx, float yy, float zz) {
	x = xx;
	y = yy;
	z = zz;
}

Point::Point(Vector3 vec) {
	x = vec.x;
	y = vec.y;
	z = vec.z;
}

bool Point::operator==(const Point& o) const {
	bool xeq = std::abs(x-o.x) < 0.001;
	bool yeq = std::abs(y-o.y) < 0.001;
	bool zeq = std::abs(z-o.z) < 0.001;
	return xeq && yeq && zeq;
}

bool Point::operator!=(const Point& o) const {
	return !(*this == o);
}

bool Point::operator<(const Point& o) const {
	bool xeq = std::abs(x-o.x) < 0.001;
	bool zeq = std::abs(z-o.z) < 0.001;
	if (!xeq) {
		return x < o.x;
	}
	if (!zeq) {
		return z < o.z;
	}
	return y < o.y;
}

Point Point::operator+(const Point& o) const {
	return {x+o.x, y+o.y, z+o.z};
}

Point Point::operator+(float n) const {
	return {x+n, y+n, z+n};
}

Point Point::operator-(const Point& o) const {
	return {x-o.x, y-o.y, z-o.z};
}

Point Point::operator-(float n) const {
	return {x-n, y-n, z-n};
}

Point Point::operator-() const {
	return {-x, -y, -z};
}

Point Point::operator*(const Point& o) const {
	return {x*o.x, y*o.y, z*o.z};
}

Point Point::operator*(float s) const {
	return {x*s, y*s, z*s};
}

void Point::operator+=(const Point& o) {
	*this = *this+o;
}

void Point::operator-=(const Point& o) {
	*this = *this-o;
}

Box Point::box() {
	float ep = std::numeric_limits<float>::epsilon() * 2;
	return (Box){x, y, z, ep, ep, ep};
}

float Point::distance(Point p) {
	return Vector3Distance(*this, p);
}

Point Point::round() {
	return (Point){
		std::round(x),
		std::round(y),
		std::round(z),
	};
}

Point Point::tileCentroid() {
	return (Point){
		std::floor(x) + 0.5f,
		std::floor(y) + 0.5f,
		std::floor(z) + 0.5f,
	};
}

Point Point::floor(float fy) {
	return (Point){x,fy,z};
}

float Point::lineDistance(Point a, Point b) {
	float ad = distance(a);
	float bd = distance(b);

	// https://gamedev.stackexchange.com/questions/72528/how-can-i-project-a-3d-point-onto-a-3d-line
	Vector3 ap = Vector3Subtract(*this, a);
	Vector3 ab = Vector3Subtract(b, a);
	Vector3 i = Vector3Add(a, Vector3Scale(ab, Vector3DotProduct(ap,ab) / Vector3DotProduct(ab,ab)));
	float d = distance(i);

	float dl = a.distance(b);

	if (dl < bd) {
		return ad;
	}

	if (dl < ad) {
		return bd;
	}

	return d;
}

Point Point::normalize() {
	return Point(Vector3Normalize(*this));
}

Point Point::cross(Point b) {
	return Point(Vector3CrossProduct(*this, b));
}

float Point::dot(Point b) {
	return Vector3DotProduct(*this, b);
}

Point Point::scale(float s) {
	return Point(Vector3Scale(*this, s));
}

Point Point::pivot(Point target, float speed) {
	Matrix r;
	target = target.normalize();
	// https://gamedev.stackexchange.com/questions/15070/orienting-a-model-to-face-a-target

	Point ahead = normalize();
	Point behind = -ahead;

	if (target == behind) {
		r = MatrixRotate(Vector3Perpendicular(ahead), 180.0f*DEG2RAD);
	}
	else
	if (target == ahead) {
		r = MatrixIdentity();
	}
	else {
		Point axis = ahead.cross(target);
		float angle = std::acos(ahead.dot(target));
		float sign = angle < 0 ? -1.0f: 1.0f;
		float delta = std::abs(angle) < speed ? angle: speed*sign;
		r = MatrixRotate(axis, delta);
	}
	return Point(Vector3Transform(*this, r));
}

Point Point::roundCardinal() {
	Point p = normalize();

	Point c = North();
	float d = p.distance(North());

	float ds = p.distance(South());
	if (ds < d) {
		c = South();
		d = ds;
	}

	float de = p.distance(East());
	if (de < d) {
		c = East();
		d = de;
	}

	float dw = p.distance(West());
	if (dw < d) {
		c = West();
		d = dw;
	}

	return c;
}

Point Point::rotateHorizontal() {
	Point p = roundCardinal();

	if (p == North()) return East();
	if (p == East()) return South();
	if (p == South()) return West();
	if (p == West()) return North();

	return p;
}

Point Point::randomHorizontal() {
	float angle = Sim::random()*360.0f*DEG2RAD;
	return transform(MatrixRotateY(angle));
}

Point Point::transform(Matrix m) {
	return Point(Vector3Transform(*this, m));
}

float Point::length() {
	return Vector3Length(*this);
}
