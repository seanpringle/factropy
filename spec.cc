#include "common.h"
#include "sim.h"
#include "spec.h"

void Spec::reset() {
	for (auto pair: all) {
		delete pair.second;
	}
	all.clear();
}

Spec::Spec(std::string name) {
	ensuref(all.count(name) == 0, "duplicate spec name %s", name.c_str());
	notef("Spec: %s", name.c_str());
	this->name = name;
	all[name] = this;

	ZERO(image);
	ZERO(texture);

	align = true;
	pivot = false;
	drone = false;
	store = false;
	belt = false;
	loader = false;
	lift = false;
	shunt = false;
	junk = false;

	magic = false;
	capacity = 0;
	enableSetLower = false;
	enableSetUpper = false;
	// loaders
	loadAnything = false;
	unloadAnything = false;
	// drones
	logistic = false;
	loadPriority = false;
	supplyPriority = false;
	defaultOverflow = false;

	place = Land;

	collision = {0,0,0};

	costGreedy = 1.0;
	clearance = 1.0;

	depot = false;
	drones = 0;

	consumeChemical = false;
	consumeElectricity = false;
	energyConsume = 0;
	generateElectricity = false;
	energyGenerate = 0;

	vehicle = false;
	vehicleEnergy = 0;
	vehicleWaitActivity = false;

	crafter = false;
	crafterProgress = false;

	teleporter = false;

	arm = false;
	armOffset = 1.0f;
}

Spec::~Spec() {
	UnloadImage(image);
}

Spec* Spec::byName(std::string name) {
	ensuref(all.count(name) == 1, "unknown spec name %s", name.c_str());
	return all[name];
}

Point Spec::aligned(Point p, Point dir) {
	if (align) {

		float ww = collision.w;
		//float hh = h;
		float dd = collision.d;

		if (dir == Point::West || dir == Point::East) {
			ww = collision.d;
			dd = collision.w;
		}

		p.x = std::floor(p.x);
		if ((int)ceil(ww)%2 != 0) {
			p.x += 0.5;
		}

		//p.y = std::floor(p.y);
		//if ((int)ceil(h)%2 != 0) {
		//	p.y += 0.5;
		//}

		p.z = std::floor(p.z);
		if ((int)ceil(dd)%2 != 0) {
			p.z += 0.5;
		}
	}
	return p;
}

Box Spec::box(Point pos, Point dir) {

	float ww = collision.w;
	float dd = collision.d;

	if (dir == Point::West || dir == Point::East) {
		ww = collision.d;
		dd = collision.w;
	}

	return {pos.x, pos.y, pos.z, ww, collision.h, dd};
}

