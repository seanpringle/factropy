#include "common.h"
#include "box.h"

Vector3 Point::vec() {
	return (Vector3){x,y,z};
}

Box Point::box() {
	float ep = std::numeric_limits<float>::epsilon() * 2;
	return (Box){x, y, z, ep, ep, ep};
}

float Point::distance(Point p) {
	return Vector3Distance(vec(), p.vec());
}