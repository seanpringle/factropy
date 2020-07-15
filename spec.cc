#include "common.h"
#include "spec.h"

Spec::Spec(std::string name) {
	ensuref(all.count(name) == 0, "duplicate spec name %s", name.c_str());
	notef("Spec: %s", name.c_str());
	this->name = name;
	all[name] = this;
}

Spec* Spec::byName(std::string name) {
	ensuref(all.count(name) == 1, "unknown spec name %s", name.c_str());
	return all[name];
}

Point Spec::aligned(Point p, enum Direction dir) {
	if (align) {
		Spec::Animation* animation = &animations[dir];

		p.x = std::floor(p.x);
		if ((int)ceil(animation->w)%2 != 0) {
			p.x += 0.5;
		}

		p.y = std::floor(p.y);
		if ((int)ceil(animation->h)%2 != 0) {
			p.y += 0.5;
		}

		p.z = std::floor(p.z);
		if ((int)ceil(animation->d)%2 != 0) {
			p.z += 0.5;
		}
	}
	return p;
}

bool Spec::hasDirection() {
	return rotate || rotateGhost;
}

