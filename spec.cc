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
	explodes = false;
	explosion = false;
	store = false;
	belt = false;
	loader = false;
	lift = false;
	shunt = false;
	junk = false;
	named = false;
	block = false;

	health = 0;
	explosionSpec = "";
	explosionDamage = 0;
	explosionRadius = 0;
	explosionRate = 0;

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
	consumeThermalFluid = false;
	energyConsume = 0;
	generateElectricity = false;
	energyGenerate = 0;

	vehicle = false;
	vehicleStop = false;
	vehicleEnergy = 0;
	vehicleWaitActivity = false;

	crafter = false;
	crafterProgress = false;

	projector = false;

	teleporter = false;

	arm = false;
	armOffset = 1.0f;

	pipe = false;
	pipeCapacity = 0;

	turret = false;
	turretRange = 0;
	turretPivot = 0;
	turretCooldown = 0;
	turretBulletSpec = "";

	missile = false;
	missileSpeed = 0;
	missileBallistic = false;

	processor = false;
	processorSpeed = 0;
	processorMarkStack = 0;
	processorDataStack = 0;
	processorReturnStack = 0;
	processorMemory = 0;
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

std::vector<Point> Spec::relativePoints(const std::vector<Point> points, const Matrix rotation, const Point position) {
	std::vector<Point> rpoints;
	for (Point point: points) {
		rpoints.push_back(point.transform(rotation) + position);
	}
	return rpoints;
}

void Spec::setCornerSupports() {
	float w = collision.w / 2.0 - 0.5;
	float d = collision.d / 2.0 - 0.5;
	float h = collision.h / 2.0 + 0.5;

	if (collision.w < 2 && collision.d < 2) {
		supportPoints.push_back(Point(0, -h, 0));
		return;
	}

	if (collision.w < 2 && collision.d > 1) {
		supportPoints.push_back(Point(0, -h, -d));
		supportPoints.push_back(Point(0, -h, +d));
		return;
	}

	if (collision.d < 2 && collision.w > 1) {
		supportPoints.push_back(Point(+w, -h, 0));
		supportPoints.push_back(Point(-w, -h, 0));
		return;
	}

	supportPoints.push_back(Point(+w, -h, -d));
	supportPoints.push_back(Point(+w, -h, +d));
	supportPoints.push_back(Point(-w, -h, -d));
	supportPoints.push_back(Point(-w, -h, +d));
}