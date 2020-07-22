#include "common.h"
#include "box.h"

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

Box Point::box() {
	float ep = std::numeric_limits<float>::epsilon() * 2;
	return (Box){x, y, z, ep, ep, ep};
}

float Point::distance(Point p) {
	return Vector3Distance(*this, p);
}

Point Point::floor(float fy) {
	return (Point){x,fy,z};
}