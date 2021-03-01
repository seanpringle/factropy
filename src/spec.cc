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
	loader = false;
	shunt = false;
	junk = false;
	named = false;
	block = false;
	rotate = false;
	toggle = false;
	enable = false;

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

	collision = {0,0,0,0,0,0};
	selection = {0,0,0,0,0,0};

	costGreedy = 1.0;
	clearance = 1.0;

	depot = false;
	drones = 0;

	consumeChemical = false;
	consumeElectricity = false;
	consumeThermalFluid = false;
	energyConsume = 0;
	energyDrain = 0;
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
	pipeUnderground = false;
	pipeUndergroundRange = 0.0f;

	turret = false;
	turretRange = 0;
	turretPivot = 0;
	turretCooldown = 0;
	turretBulletSpec = "";

	missile = false;
	missileSpeed = 0;
	missileBallistic = false;

	conveyor = false;
	unveyor = false;
	unveyorRange = 0.0f;
	unveyorEntry = false;

	ropeway = false;
	ropewayTower = false;
	ropewayTerminus = false;
	ropewayCableEast = Point::Zero;
	ropewayBucket = false;
	ropewayBucketSpec = nullptr;

	computer = false;
	networker = false;

	cycle = nullptr;
	pipette = nullptr;

	build = true;
	select = true;

	statsGroup = this;
	energyConsumption.clear();
}

Spec::~Spec() {
	UnloadImage(image);
}

Spec* Spec::byName(std::string name) {
	ensuref(all.count(name) == 1, "unknown spec name %s", name.c_str());
	return all[name];
}

Point Spec::aligned(Point p, Point dir) {
	p += collision.centroid();
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

Box Spec::box(Point pos, Point dir, Box vol) {
	pos += vol.centroid();

	float ww = vol.w;
	float dd = vol.d;

	if (dir == Point::West || dir == Point::East) {
		ww = vol.d;
		dd = vol.w;
	}

	return {pos.x, pos.y, pos.z, ww, vol.h, dd};
}

Box Spec::southBox(Point pos) {
	pos += collision.centroid();
	return {pos.x, pos.y, pos.z, collision.w, collision.h, collision.d};
}

std::vector<Point> Spec::relativePoints(const std::vector<Point> points, const Matrix rotation, const Point position) {
	std::vector<Point> rpoints;
	for (Point point: points) {
		rpoints.push_back(point.transform(rotation) + position);
	}
	return rpoints;
}
