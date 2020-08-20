#include "common.h"
#include "vehicle.h"
#include "sim.h"

void Vehicle::reset() {
	all.clear();
}

void Vehicle::tick() {
	for (auto& vehicle: all) {
		vehicle.update();
	}
}

Vehicle& Vehicle::create(int id) {
	Vehicle& vehicle = all.ref(id);
	vehicle.id = id;
	vehicle.pause = 0;
	vehicle.patrol = false;
	vehicle.waypoint = NULL;
	vehicle.handbrake = false;
	vehicle.pathRequest = NULL;
	return vehicle;
}

Vehicle& Vehicle::get(int id) {
	ensuref(all.has(id), "invalid vehicle access %d", id);
	return all.ref(id);
}

void Vehicle::destroy() {
	if (pathRequest) {
		Path::jobs.remove(pathRequest);
		delete pathRequest;
		pathRequest = NULL;
	}
	all.drop(id);
}

void Vehicle::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;
	if (handbrake || pause > Sim::tick) return;

	if (path.empty() && waypoint) {
		for (auto condition: waypoint->conditions) {
			if (!condition->ready(waypoint, this)) {
				return;
			}
		}
		waypoint = NULL;
	}

	// check an existing pathfinding request for completion
	if (pathRequest && pathRequest->done) {

		if (waypoints.size()) {
			waypoint = waypoints.front();
			waypoints.pop_front();
			ensure(waypoint);

			if (patrol) {
				waypoints.push_back(waypoint);
			} else {
				delete waypoint;
				waypoint = NULL;
			}
		}

		if (pathRequest->success) {
			notef("path success");
			for (Point point: pathRequest->result) {
				path.push_back(point);
			}
		}
		else {
			notef("path failure");
		}

		delete pathRequest;
		pathRequest = NULL;
	}

	// request pathfinding for the next leg of our route
	if (!pathRequest && path.empty() && !waypoints.empty()) {
		pathRequest = new Route(this);
		pathRequest->origin = Point(en.pos.x, 0.0f, en.pos.z).tileCentroid();
		pathRequest->target = waypoints.front()->position;
		pathRequest->submit();
		notef("send path request %f,%f,%f", pathRequest->target.x, pathRequest->target.y, pathRequest->target.z);
		return;
	}

	if (path.empty()) {
		pause = Sim::tick + 15;
		return;
	}

	Point centroid = en.pos.floor(0);
	Point target = path.front().floor(0);

	if (!en.lookAtPivot({path.front().x, en.pos.y, path.front().z})) {
		return;
	}

	float fueled = en.consume(en.spec->vehicleEnergy).portion(en.spec->vehicleEnergy);

	float distance = target.distance(centroid);
	float speed = std::max(0.01f, 0.1f * fueled);

	bool arrived = distance < speed;

	if (arrived) {
		en.move(target).floor(0);
		path.pop_front();
		if (!path.size()) {
			pause = Sim::tick + 120;
		}
		return;
	}

	Point v = target - centroid;
	Point n = v.normalize();
	Point s = n * speed;

	en.move(centroid + s).floor(0);
}

Vehicle::Waypoint* Vehicle::addWaypoint(Point p) {
	if (!patrol) {
		path.clear();
	}
	p.y = 0.0f;
	waypoints.push_back(
		new Waypoint(p.tileCentroid())
	);
	return waypoints.back();
}

Vehicle::Waypoint* Vehicle::addWaypoint(uint eid) {
	if (!patrol) {
		path.clear();
	}
	waypoints.push_back(
		new Waypoint(eid)
	);
	return waypoints.back();
}

Vehicle::Route::Route(Vehicle *v) : Path() {
	vehicle = v;
}

std::vector<Point> Vehicle::Route::getNeighbours(Point p) {
	p = p.tileCentroid();
	float clearance = Entity::get(vehicle->id).spec->clearance;

	auto entities = Entity::intersecting(p.box().grow(clearance));

	std::vector<Point> points = {
		(p + Point( 1.0f, 0.0f, 0.0f)).tileCentroid(),
		(p + Point(-1.0f, 0.0f, 0.0f)).tileCentroid(),
		(p + Point( 0.0f, 0.0f, 1.0f)).tileCentroid(),
		(p + Point( 0.0f, 0.0f,-1.0f)).tileCentroid(),
		(p + Point( 1.0f, 0.0f, 1.0f)).tileCentroid(),
		(p + Point(-1.0f, 0.0f, 1.0f)).tileCentroid(),
		(p + Point( 1.0f, 0.0f,-1.0f)).tileCentroid(),
		(p + Point(-1.0f, 0.0f,-1.0f)).tileCentroid(),
	};

	for (int eid: entities) {
		if (Entity::get(eid).spec->vehicleStop) {
			continue;
		}
		if (vehicle->id != eid) {
			Entity& en = Entity::get(eid);
			Box box = en.box();
			for (auto it = points.begin(); it != points.end(); ) {
				if (box.contains(*it)) {
					points.erase(it);
					continue;
				}
				it++;
			}
		}
	}

	float range = origin.distance(target);

	for (auto it = points.begin(); it != points.end(); ) {
		if (!Chunk::isLand(it->box().grow(clearance))) {
			points.erase(it);
			continue;
		}
		// capsule shaped range limit
		if ((*it).lineDistance(origin, target) > range) {
			points.erase(it);
			continue;
		}
		it++;
	}

	for (auto it = points.begin(); it != points.end(); ) {
		if (origin.tileCentroid() == *it) {
			*it = origin;
		}
		if (target.tileCentroid() == *it) {
			*it = target;
		}
		it++;
	}

	return points;
}

double Vehicle::Route::calcCost(Point a, Point b) {
	float clearance = Entity::get(vehicle->id).spec->clearance;
	float cost = a.distance(b);

	if (!Chunk::isLand(b.box().grow(clearance))) {
		cost *= 1000.0f;
	}

	auto entities = Entity::intersecting(b.box().grow(clearance, 0, clearance));

	for (int eid: entities) {
		if (Entity::get(eid).spec->vehicleStop) {
			continue;
		}
		if (vehicle->id != eid) {
			cost *= 5.0f;
		}
	}

	return cost;
}

double Vehicle::Route::calcHeuristic(Point p) {
	return p.distance(target) * Entity::get(vehicle->id).spec->costGreedy;
}

bool Vehicle::Route::rayCast(Point a, Point b) {
	float clearance = Entity::get(vehicle->id).spec->clearance;

	float d = a.distance(b);
	if (d > 32.0f) return false;

	Point n = (b-a).normalize();

	for (Point c = a; c.distance(b) > 1.0f; c += n) {
		if (!Chunk::isLand(c.box().grow(clearance))) {
			return false;
		}
		for (int eid: Entity::intersecting(c.box().grow(1))) {
			if (Entity::get(eid).spec->vehicleStop) {
				continue;
			}
			if (vehicle->id != eid) {
				return false;
			}
		}
	}

	return true;
}

Vehicle::Waypoint::Waypoint(Point pos) {
	position = pos;
	stopId = 0;
	stopName = "";
}

Vehicle::Waypoint::Waypoint(uint eid) {
	Entity& en = Entity::get(eid);
	position = en.ground();
	stopId = en.id;
	stopName = en.name();
}

Vehicle::Waypoint::~Waypoint() {
	for (auto c: conditions) {
		delete c;
	}
	conditions.clear();
}

Vehicle::DepartCondition::~DepartCondition() {
}

Vehicle::DepartInactivity::~DepartInactivity() {
}

bool Vehicle::DepartInactivity::ready(Waypoint* waypoint, Vehicle *vehicle) {
	Entity& en = Entity::get(vehicle->id);
	return !en.spec->store || en.store().activity < Sim::tick-(uint)(seconds*60);
}

Vehicle::DepartItem::~DepartItem() {
}

bool Vehicle::DepartItem::ready(Waypoint* waypoint, Vehicle *vehicle) {
	Entity& en = Entity::get(vehicle->id);

	if (en.spec->store) {
		switch (op) {
			case Eq: {
				return en.store().count(iid) == count;
			}
			case Ne: {
				return en.store().count(iid) != count;
			}
			case Lt: {
				return en.store().count(iid) < count;
			}
			case Lte: {
				return en.store().count(iid) <= count;
			}
			case Gt: {
				return en.store().count(iid) > count;
			}
			case Gte: {
				return en.store().count(iid) >= count;
			}
		}
	}

	return true;
}