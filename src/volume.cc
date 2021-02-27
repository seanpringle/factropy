#include "common.h"
#include "point.h"
#include "volume.h"

Volume::Volume() {
	w = 0.0f;
	h = 0.0f;
	d = 0.0f;
}

Volume::Volume(std::initializer_list<float> l) {
	auto i = l.begin();
	w = *i++;
	h = *i++;
	d = *i++;
}

Volume::Volume(float ww, float hh, float dd) {
	w = ww;
	h = hh;
	d = dd;
}

Volume::operator bool() const {
	return Point(w, h, d).length() > 0.01;
}