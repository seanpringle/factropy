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
	return vehicle;
}

Vehicle& Vehicle::get(int id) {
	ensuref(all.has(id), "invalid vehicle access %d", id);
	return all.ref(id);
}

void Vehicle::destroy() {
	all.drop(id);

	if (pathRequest) {
		Path::jobs.remove(pathRequest);
		delete pathRequest;
		pathRequest = NULL;
	}
}

void Vehicle::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;
	if (pause > Sim::tick) return;

	// check an existing pathfinding request for completion
	if (pathRequest && pathRequest->done) {
		
		if (pathRequest->success) {
			notef("path success");
			for (Point point: pathRequest->result) {
				path.push_back(point);
			}
		} else {
			notef("path failure");
		}
		
		delete pathRequest;
		pathRequest = NULL;
	}

	// request pathfinding for the next leg of our route
	if (!pathRequest && path.empty() && !legs.empty()) {

		pathRequest = new Route(this);
		pathRequest->origin = Point(en.pos.x, 0.0f, en.pos.z).tileCentroid();
		pathRequest->target = legs.front();
		legs.pop_front();
		pathRequest->submit();
		notef("send path request");
		return;
	}


	if (path.empty()) {
		pause = Sim::tick + 15;
		return;
	}

	Point centroid = en.pos.floor(0);
	Point target = path.front().floor(0);

	if (!en.lookAtPivot((Vector3){path.front().x, en.pos.y, path.front().z})) {
		return;
	}

	float distance = target.distance(centroid);
	float speed = 0.1f;

	bool arrived = distance < speed;

	if (arrived) {
		en.move(target).floor(0);
		path.pop_front();
		return;
	}

	Vector3 v = target - centroid;
	Vector3 n = Vector3Normalize(v);
	Vector3 s = Vector3Scale(n, speed);

	en.move(centroid + Point(s)).floor(0);
}

void Vehicle::addWaypoint(Point p) {
	path.clear(); // temp
	p.y = 0.0f;
	legs.push_back(p.tileCentroid());
}

Vehicle::Route::Route(Vehicle *v) : Path() {
	vehicle = v;
}

std::vector<Point> Vehicle::Route::getNeighbours(Point p) {
	p = p.tileCentroid();

	auto entities = Entity::intersecting(p.box().grow(1.5, 0.5, 1.5));

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
		Chunk::Tile *tile = Chunk::tileTryGet(*it);
		if (!tile || tile->elevation > 0.01 || tile->elevation < -0.01) {
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
	return a.distance(b);
}

double Vehicle::Route::calcHeuristic(Point p) {
	return p.distance(target);
}

bool Vehicle::Route::rayCast(Point a, Point b) {
	return false;
}
