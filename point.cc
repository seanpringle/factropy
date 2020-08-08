#include "common.h"
#include "box.h"
#include "sim.h"

const Point Point::Zero  = { 0, 0, 0};
const Point Point::North = { 0, 0,-1};
const Point Point::South = { 0, 0, 1};
const Point Point::East  = { 1, 0, 0};
const Point Point::West  = {-1, 0, 0};
const Point Point::Up    = { 0, 1, 0};
const Point Point::Down  = { 0,-1, 0};

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

Point::Point(Volume vol) {
	x = vol.w;
	y = vol.h;
	z = vol.d;
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

Box Point::box() const {
	float ep = std::numeric_limits<float>::epsilon() * 2;
	return (Box){x, y, z, ep, ep, ep};
}

float Point::distance(Point p) const {
	return Vector3Distance(*this, p);
}

Point Point::round() const {
	return (Point){
		std::round(x),
		std::round(y),
		std::round(z),
	};
}

Point Point::tileCentroid() const {
	return (Point){
		std::floor(x) + 0.5f,
		std::floor(y) + 0.5f,
		std::floor(z) + 0.5f,
	};
}

Point Point::floor(float fy) const {
	return (Point){x,fy,z};
}

float Point::lineDistance(Point a, Point b) const {
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

Point Point::normalize() const {
	return Point(Vector3Normalize(*this));
}

Point Point::cross(Point b) const {
	return Point(Vector3CrossProduct(*this, b));
}

float Point::dot(Point b) const {
	return Vector3DotProduct(*this, b);
}

Point Point::scale(float s) const {
	return Point(Vector3Scale(*this, s));
}

Point Point::pivot(Point target, float speed) const {
	Mat4 r;
	target = target.normalize();
	// https://gamedev.stackexchange.com/questions/15070/orienting-a-model-to-face-a-target

	Point ahead = normalize();
	Point behind = -ahead;

	if (target == behind) {
		r = Mat4::rotate(Vector3Perpendicular(ahead), 180.0f*DEG2RAD);
	}
	else
	if (target == ahead) {
		r = Mat4::identity;
	}
	else {
		Point axis = ahead.cross(target);
		float angle = std::acos(ahead.dot(target));
		float sign = angle < 0 ? -1.0f: 1.0f;
		float delta = std::abs(angle) < speed ? angle: speed*sign;
		r = Mat4::rotate(axis, delta);
	}
	return Point(Vector3Transform(*this, r));
}

Point Point::roundCardinal() const {
	Point p = normalize();

	Point c = North;
	float d = p.distance(North);

	float ds = p.distance(South);
	if (ds < d) {
		c = South;
		d = ds;
	}

	float de = p.distance(East);
	if (de < d) {
		c = East;
		d = de;
	}

	float dw = p.distance(West);
	if (dw < d) {
		c = West;
		d = dw;
	}

	return c;
}

Point Point::rotateHorizontal() const {
	Point p = roundCardinal();

	if (p == North) return East;
	if (p == East) return South;
	if (p == South) return West;
	if (p == West) return North;

	return p;
}

Point Point::randomHorizontal() const {
	float angle = Sim::random()*360.0f*DEG2RAD;
	return transform(Mat4::rotateY(angle));
}

Point Point::transform(Mat4 m) const {
	return Point(Vector3Transform(*this, m));
}

float Point::length() const {
	return Vector3Length(*this);
}

Mat4 Point::rotation() {
	Mat4 r = Mat4::identity;
	Point dir = *this;

	// https://gamedev.stackexchange.com/questions/15070/orienting-a-model-to-face-a-target
	if (dir == Point::North) {
		r = Mat4::rotate(Point::Up, 180.0f*DEG2RAD);
	}
	else
	if (dir == Point::South) {
		r = Mat4::identity;
	}
	else {
		Point axis = Point::South.cross(dir);
		float angle = std::acos(Point::South.dot(dir));
		r = Mat4::rotate(axis, angle);
	}

	return r;
}