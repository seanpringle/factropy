#include "common.h"
#include "point.h"
#include "area.h"

Area::Area() {
	w = 0.0f;
	d = 0.0f;
}

Area::Area(std::initializer_list<float> l) {
	auto i = l.begin();
	w = *i++;
	d = *i++;
}

Area::Area(float ww, float dd) {
	w = ww;
	d = dd;
}

Area::operator bool() const {
	return Point(w, 0, d).length() > 0.01;
}

Area Area::direction(Point dir) {
	return (dir == Point::West || dir == Point::East) ? Area(d,w): Area(w,d);
}
