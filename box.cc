#include "common.h"
#include "box.h"

Point Box::centroid() {
	return (Point){x, y, z};
}

BoundingBox Box::bounds() {
	return (BoundingBox){
		(Vector3){x - w/2, y - h/2, z - d/2},
		(Vector3){x + w/2, y + h/2, z + d/2},
	};
}

Box Box::translate(Point p) {
	return (Box){x + p.x, y + p.y, z + p.z, w, h, d};
}

Box Box::grow(Point p) {
	return (Box){x, y, z, w + p.x*2, h + p.y*2, d + p.z*2};
}

Box Box::shrink(Point p) {
	return grow((Point){-p.x, -p.y, -p.z});
}

bool Box::intersects(Box a) {
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