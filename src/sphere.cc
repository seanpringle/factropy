#include "common.h"
#include "sphere.h"

Sphere::Sphere() {
	x = 0;
	y = 0;
	z = 0;
	r = 0;
}

Sphere::Sphere(Point p, float radius) {
	x = p.x;
	y = p.y;
	z = p.z;
	r = radius;
}

Sphere::Sphere(float px, float py, float pz, float radius) {
	x = px;
	y = py;
	z = pz;
	r = radius;
}

Sphere::Sphere(std::initializer_list<float> l) {
	auto i = l.begin();
	x = *i++;
	y = *i++;
	z = *i++;
	r = *i++;
}

Point Sphere::centroid() const {
	return (Point){x, y, z};
}

Sphere Sphere::grow(float n) const {
	float ep = std::numeric_limits<float>::epsilon() * 2;
	return Sphere(centroid(), std::max(r+n, ep));
}

Sphere Sphere::shrink(float n) const {
	return grow(-n);
}

Sphere Sphere::translate(Point p) const {
	return Sphere(centroid()+p, r);
}

Sphere Sphere::translate(float xx, float yy, float zz) const {
	return translate(Point(xx,yy,zz));
}

bool Sphere::intersects(Sphere a) const {
	return centroid().distance(a.centroid()) < r+a.r;
}

bool Sphere::contains(Point p) const {
	return centroid().distance(p) < r;
}
