#include "common.h"
#include "entity.h"
#include "drone.h"

void Drone::reset() {
	all.clear();
}

void Drone::tick() {
	for (auto& drone: all) {
		drone.update();
	}
}

Drone& Drone::create(uint id) {
	Drone& drone = all[id];
	drone.id = id;
	drone.iid = 0;
	drone.dep = 0;
	drone.src = 0;
	drone.dst = 0;
	drone.srcGhost = false;
	drone.dstGhost = false;
	drone.stage = Stranded;
	return drone;
}

Drone& Drone::get(uint id) {
	return all.refer(id);
}

void Drone::destroy() {
	if (dep && Entity::exists(dep)) {
		Entity::get(dep).depot().drones.erase(id);
	}
	all.erase(id);
}

bool Drone::travel(uint eid) {
	Entity& en = Entity::get(id);
	Entity& te = Entity::get(eid);

	Point arrivalPoint = te.pos + (Point::Up * te.spec->collision.h);
	Point arrivalDir = (arrivalPoint - en.pos).normalize();
	float arrivalDist = en.pos.distance(arrivalPoint);

	Point overheadPoint = {arrivalPoint.x, en.pos.y, arrivalPoint.z};
	Point overheadDir = (overheadPoint - en.pos).normalize();
	//float overheadDist = en.pos.distance(overheadPoint);

	float speed = en.spec->droneSpeed;

	en.lookAt(overheadPoint);

	Point arrivalStep = en.pos + (arrivalDir*speed);
	Point overheadStep = en.pos + (overheadDir*speed);

	if (arrivalDist < speed*1.1f) {
		en.move(arrivalPoint);
		return false;
	}

	if (arrivalDist < speed*60.0f) {
		en.move(en.pos + arrivalStep);
		return false;
	}

	std::function<bool(Point, Point)> rayCast = [&](Point start, Point end) {
		auto hits = Entity::intersecting(Box(start, end));

		discard_if(hits, [&](uint cid) {
			return cid == id || cid == src || cid == dst || cid == dep || all.has(cid);
		});

		if (!hits.size()) return true;

		std::vector<Box> boxes;
		for (auto cid: hits) {
			boxes.push_back(Entity::get(cid).box());
		}

		float step = 0.5f;
		Point direction = (end-start).normalize();
		int steps = (int)std::floor(start.distance(end)/step);

		for (int i = 0; i < steps; i++) {
			float offset = step*(float)i;
			Point stepPoint = start + (direction*offset);
			Box stepBox = stepPoint.box().grow(step);
			for (auto box: boxes) {
				if (box.intersects(stepBox)) {
					return false;
				}
			}
		}
		return true;
	};

	bool arrivalVisibleNext = rayCast(en.pos + arrivalStep, arrivalPoint);

	if (arrivalVisibleNext) {
		en.move(en.pos + arrivalStep);
		return false;
	}

	bool overheadVisibleNext = rayCast(en.pos + overheadStep, overheadPoint);

	if (overheadVisibleNext) {
		en.move(en.pos + overheadStep);
		return false;
	}

	en.move(en.pos + overheadDir*speed + Point::Up*speed);
	return true;
}

void Drone::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;

	switch (stage) {
		case ToSrc: {
			if (!Entity::exists(src) || Entity::get(src).isGhost() != srcGhost) {
				stage = ToDep;
				break;
			}

			if (!travel(src)) {
				Entity& se = Entity::get(src);
				Store& store = srcGhost ? se.ghost().store: se.store();
				store.remove(stack);
				store.drones.erase(id);
				stage = ToDst;
				iid = stack.iid;
				break;
			}

			break;
		}

		case ToDst: {
			if (!Entity::exists(dst) || Entity::get(dst).isGhost() != dstGhost) {
				stage = ToDep;
				break;
			}

			if (!travel(dst)) {
				Entity& de = Entity::get(dst);
				Store& store = dstGhost ? de.ghost().store: de.store();
				store.insert(stack);
				store.drones.erase(id);
				stage = ToDep;
				iid = 0;
				break;
			}

			break;
		}

		case ToDep: {
			if (!Entity::exists(dep)) {
				stage = Stranded;
				break;
			}

			if (!travel(dep)) {
				en.remove();
				break;
			}

			break;
		}

		case Stranded: {
			break;
		}
	}
}
