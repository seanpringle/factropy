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

Sphere Point::sphere() const {
	float ep = std::numeric_limits<float>::epsilon() * 2;
	return Sphere(x, y, z, ep);
}

float Point::distanceSquared(Point p) const {
  float dx = p.x - x;
  float dy = p.y - y;
  float dz = p.z - z;
  return dx*dx + dy*dy + dz*dz;
}

float Point::distance(Point p) const {
	return std::sqrt(distanceSquared(p));
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

Point Point::oppositeCardinal() const {
	return rotateHorizontal().rotateHorizontal();
}

bool Point::cardinalParallel(Point p) const {
	return
		((*this == North || *this == South) && (p == North || p == South)) ||
		((*this == East  || *this == West ) && (p == East  || p == West ));
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
	if (dir == Point::East) {
		r = Mat4::rotate(Point::Up, 90.0f*DEG2RAD);
	}
	else
	if (dir == Point::West) {
		r = Mat4::rotate(Point::Up, 270.0f*DEG2RAD);
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

Mat4 Point::translation() {
	return Mat4::translate(*this);
}

Point Point::nearestPointOnLine(Point start, Point end) const {
	auto line = (end - start);
	float len = line.length();
	auto nline = line.normalize();

	auto v = *this - start;
	float d = v.dot(nline);
	d = std::max(0.0f, std::min(len, d));
	return start + nline * d;
}

bool
Point::linesCrossOnGround(Point a0, Point a1, Point b0, Point b1) //, float *i_x, float *i_y)
{
	// https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
	float ax0 = a0.x;
	float ay0 = a0.z;
	float ax1 = a1.x;
	float ay1 = a1.z;
	float bx0 = b0.x;
	float by0 = b0.z;
	float bx1 = b1.x;
	float by1 = b1.z;

	float s02_x, s02_y, s10_x, s10_y, s32_x, s32_y, s_numer, t_numer, denom; //, t;

	s10_x = ax1 - ax0;
	s10_y = ay1 - ay0;
	s32_x = bx1 - bx0;
	s32_y = by1 - by0;

	denom = s10_x * s32_y - s32_x * s10_y;
	if (denom == 0)
		return 0; // Collinear
	bool denomPositive = denom > 0;

	s02_x = ax0 - bx0;
	s02_y = ay0 - by0;
	s_numer = s10_x * s02_y - s10_y * s02_x;

	if ((s_numer < 0) == denomPositive)
		return 0; // No collision

	t_numer = s32_x * s02_y - s32_y * s02_x;

	if ((t_numer < 0) == denomPositive)
		return 0; // No collision

	if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive))
		return 0; // No collision

	// Collision detected
	//t = t_numer / denom;
	//if (i_x != NULL)
	//    *i_x = ax0 + (t * s10_x);
	//if (i_y != NULL)
	//    *i_y = ay0 + (t * s10_y);

	return 1;
}