#include "common.h"
#include "entity.h"
#include "drone.h"

void Drone::reset() {
	all.clear();
}

void Drone::tick() {
	for (Drone& drone: all) {
		drone.update();
	}
}

Drone& Drone::create(uint id) {
	Drone& drone = all.ref(id);
	drone.id = id;
	drone.dep = 0;
	drone.src = 0;
	drone.dst = 0;
	drone.srcGhost = false;
	drone.dstGhost = false;
	drone.stage = Stranded;
	return drone;
}

Drone& Drone::get(uint id) {
	ensuref(all.has(id), "invalid drone access %d", id);
	return all.ref(id);
}

void Drone::destroy() {
	if (dep && Entity::exists(dep)) {
		Entity::get(dep).depot().drones.erase(id);
	}
	all.drop(id);
}

bool Drone::travel(uint eid) {
	Entity& en = Entity::get(id);
	Entity& te = Entity::get(eid);
	Point tp = te.pos + Point::Up*2;

	if (en.pos.distance(tp) < 0.11) {
		en.move(tp);
		return false;
	}

	Point dir = tp - en.pos;
	en.move(en.pos + dir.normalize()*0.1);
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
