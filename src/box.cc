#include "common.h"
#include "box.h"

Box::Box() {
	x = 0;
	y = 0;
	z = 0;
	w = 0;
	h = 0;
	d = 0;
}

Box::Box(Point p, Volume v) {
	x = p.x;
	y = p.y;
	z = p.z;
	w = v.w;
	h = v.h;
	d = v.d;
}

Box::Box(Point a, Point b) {
	Point c = a + ((b - a) * 0.5f);
	x = c.x;
	y = c.y;
	z = c.z;
	w = std::abs(b.x - a.x);
	h = std::abs(b.y - a.y);
	d = std::abs(b.z - a.z);
}

Box::Box(std::initializer_list<float> l) {
	auto i = l.begin();
	x = *i++;
	y = *i++;
	z = *i++;
	w = *i++;
	h = *i++;
	d = *i++;
}

Point Box::centroid() const {
	return (Point){x, y, z};
}

Point Box::dimensions() const {
	return (Point){w, h, d};
}

BoundingBox Box::bounds() const {
	return (BoundingBox){
		(Vector3){x - w/2, y - h/2, z - d/2},
		(Vector3){x + w/2, y + h/2, z + d/2},
	};
}

Box Box::translate(Point p) const {
	return (Box){x + p.x, y + p.y, z + p.z, w, h, d};
}

Box Box::translate(float xx, float yy, float zz) const {
	return translate(Point(xx,yy,zz));
}

Box Box::grow(Point p) const {
	return (Box){x, y, z, w + p.x*2, h + p.y*2, d + p.z*2};
}

Box Box::grow(float n) const {
	return grow(Point(n,n,n));
}

Box Box::grow(float xx, float yy, float zz) const {
	return grow(Point(xx,yy,zz));
}

Box Box::shrink(Point p) const {
	return grow((Point){-p.x, -p.y, -p.z});
}

Box Box::shrink(float n) const {
	return shrink(Point(n,n,n));
}

Box Box::shrink(float xx, float yy, float zz) const {
	return shrink(Point(xx,yy,zz));
}

bool Box::intersects(Box a) const {
	float xMin = x-w/2.0f;
	float yMin = y-h/2.0f;
	float zMin = z-d/2.0f;
	float xMax = x+w/2.0f;
	float yMax = y+h/2.0f;
	float zMax = z+d/2.0f;

	float axMin = a.x-a.w/2.0f;
	float ayMin = a.y-a.h/2.0f;
	float azMin = a.z-a.d/2.0f;
	float axMax = a.x+a.w/2.0f;
	float ayMax = a.y+a.h/2.0f;
	float azMax = a.z+a.d/2.0f;

	return (axMin <= xMax && axMax >= xMin) && (ayMin <= yMax && ayMax >= yMin) && (azMin <= zMax && azMax >= zMin);
}

bool Box::contains(Point p) const {
	float xMin = x-w/2.0f - 0.000001f;
	float yMin = y-h/2.0f - 0.000001f;
	float zMin = z-d/2.0f - 0.000001f;
	float xMax = x+w/2.0f + 0.000001f;
	float yMax = y+h/2.0f + 0.000001f;
	float zMax = z+d/2.0f + 0.000001f;

	bool xc = xMin < p.x && xMax > p.x;
	bool yc = yMin < p.y && yMax > p.y;
	bool zc = zMin < p.z && zMax > p.z;

	return xc && yc && zc;
}

float Box::volume() const {
	return w*d*h;
}

Sphere Box::sphere() const {
	float r = Point(w/2.0f, h/2.0f, d/2.0f).length();
	return Sphere(x, y, z, r);
}