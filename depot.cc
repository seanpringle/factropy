#include "common.h"
#include "depot.h"
#include "sim.h"

void Depot::reset() {
	all.clear();
}

void Depot::tick() {
	for (auto& depot: all) {
		depot.update();
	}
}

Depot& Depot::create(uint id) {
	Depot& depot = all[id];
	depot.id = id;
	depot.pause = 0;
	return depot;
}

Depot& Depot::get(uint id) {
	ensuref(all.has(id) > 0, "invalid depot access %d", id);
	return all[id];
}

void Depot::destroy() {
	all.erase(id);
}

void Depot::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;
	if (pause > Sim::tick) return;
	if (drones.size() >= en.spec->drones) return;

	Box range = en.pos.box().grow(32);
	auto entities = Entity::intersecting(range);

	for (uint eid: entities) {
		Entity& se = Entity::get(eid);

		if (se.isConstruction()) {
			Stack stack = se.ghost().store.forceSupplyFrom(en.store());
			if (stack.size) {
				dispatch(id, id, se.id, stack);
				return;
			}
		}
	}

	for (uint eid: entities) {
		Entity& se = Entity::get(eid);

		if (se.isDeconstruction()) {
			Stack stack = se.ghost().store.forceOverflowTo(en.store());
			if (stack.size) {
				dispatch(id, se.id, id, stack);
				return;
			}
		}
	}

	for (uint eid: entities) {
		Entity& se = Entity::get(eid);

		if (se.spec->store && se.spec->logistic) {
			Stack stack = en.store().supplyFrom(se.store());
			if (stack.size) {
				dispatch(id, se.id, id, stack);
				return;
			}
		}
	}

	for (uint eid: entities) {
		Entity& se = Entity::get(eid);

		if (se.spec->store && se.spec->logistic) {
			Stack stack = en.store().overflowTo(se.store());
			if (stack.size) {
				dispatch(id, id, se.id, stack);
				return;
			}
		}
	}
}

void Depot::dispatch(uint dep, uint src, uint dst, Stack stack) {
	Entity& en = Entity::get(id);

	Entity& ed = Entity::create(Entity::next(), Spec::byName("drone"));
	ed.move(en.pos + Point::Up*2);
	ed.materialize();
	drones.insert(ed.id);

	Drone& drone = ed.drone();
	drone.dep = dep;
	drone.src = src;
	drone.dst = dst;
	drone.stack = stack;
	drone.srcGhost = Entity::get(src).isGhost();
	drone.dstGhost = Entity::get(dst).isGhost();
	drone.stage = Drone::ToSrc;

	Entity &se = Entity::get(src);
	Store& ss = drone.srcGhost ? se.ghost().store: se.store();
	ss.reserve(stack);
	ss.drones.insert(ed.id);

	Entity &de = Entity::get(dst);
	Store& ds = drone.dstGhost ? de.ghost().store: de.store();
	ds.promise(stack);
	ds.drones.insert(ed.id);

	pause = Sim::tick + 30;
}
