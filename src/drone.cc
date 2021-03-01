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
	Point tp = te.pos + (Point::Up * te.spec->collision.h);

	en.lookAt({tp.x, en.pos.y, tp.z});

	if (en.pos.distance(tp) < 0.11) {
		en.move(tp);
		return false;
	}

	float speed = 0.1f;

	Point dir = (tp - en.pos).normalize();
	Point step = en.pos + (dir*speed);

	auto hits = Entity::intersecting(Box(en.pos, te.pos));

	for (int i = 0, l = (int)std::ceil(en.pos.distance(tp)/speed); i < l; i++) {
		Point p = en.pos + (dir*(speed*(float)i));
		for (auto cid: hits) {
			if (cid == id || cid == src || cid == dst || cid == dep) continue;
			if (all.has(cid)) continue; // drone
			if (Entity::get(cid).box().intersects((p+Point::Down).box().grow(0.5f))) {
				step = en.pos + en.dir*speed + Point::Up*speed;
				i = l;
				break;
			}
		}
	}

	en.move(step);
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
