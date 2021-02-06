#include "common.h"
#include "sim.h"
#include "conveyor.h"

void Conveyor::reset() {
	all.clear();
}

void Conveyor::tick() {
	for (auto& pair: all) {
		if (!pair.second.next) {
			pair.second.update();
		}
	}
	for (auto& pair: all) {
		if (pair.second.ticked < Sim::tick) {
			pair.second.update();
		}
	}
}

Conveyor& Conveyor::create(uint id) {
	Entity& en = Entity::get(id);
	Conveyor& conveyor = all[id];
	conveyor.id = id;
	conveyor.iid = 0;
	conveyor.offset = 0;
	conveyor.steps = en.spec->conveyorTransforms.size();
	conveyor.prev = 0;
	conveyor.next = 0;
	conveyor.ticked = 0;
	return conveyor;
}

Conveyor& Conveyor::get(uint id) {
	ensuref(all.count(id), "invalid conveyor access %d", id);
	return all[id];
}

void Conveyor::destroy() {
	if (prev || next) {
		unmanage();
	}
	all.erase(id);
}

Point Conveyor::input() {
	Entity& en = Entity::get(id);
	return en.spec->conveyorInput
		.transform(en.dir.rotation())
		.transform(en.pos.translation())
	;
}

Point Conveyor::output() {
	Entity& en = Entity::get(id);
	return en.spec->conveyorOutput
		.transform(en.dir.rotation())
		.transform(en.pos.translation())
	;
}

Conveyor& Conveyor::manage() {
	ensure(!prev);
	ensure(!next);

	Entity& en = Entity::get(id);

	Box ib = input().box().grow(0.1f);
	Box ob = output().box().grow(0.1f);

	for (auto oid: Entity::intersecting(ob)) {
		ensure(oid != id);
		Entity& eo = Entity::get(oid);
		if (eo.isGhost()) continue;
		if (eo.spec->conveyor) {
			Conveyor& co = eo.conveyor();
			if (en.box().contains(co.input())) {
				ensure(!co.prev);
				co.prev = id;
				next = oid;
				break;
			}
		}
	}

	for (auto oid: Entity::intersecting(ib)) {
		ensure(oid != id);
		Entity& ei = Entity::get(oid);
		if (ei.isGhost()) continue;
		if (ei.spec->conveyor) {
			Conveyor& ci = ei.conveyor();
			if (en.box().contains(ci.output())) {
				ensure(!ci.next);
				ci.next = id;
				prev = oid;
				break;
			}
		}
	}

	return *this;
}

Conveyor& Conveyor::unmanage() {

	if (prev) {
		ensure(get(prev).next == id);
		get(prev).next = 0;
		prev = 0;
	}

	if (next) {
		ensure(get(next).prev == id);
		get(next).prev = 0;
		next = 0;
	}

	return *this;
}

bool Conveyor::deliver(uint iiid) {
	if (!iid || !prev) {
		iid = iiid;
		offset = steps-1;
		return true;
	}
	return false;
}

void Conveyor::update() {
	if (Sim::tick == ticked) {
		return;
	}

	ticked = Sim::tick;

	for(;;) {
		if (!iid) {
			break;
		}

		uint nextOffset = next ? get(next).offset: 0;

		if (next && nextOffset && offset <= nextOffset) {
			break;
		}

		if (!next) {
			if (offset <= steps/2) {
				break;
			}
		}

		if (offset > 0) {
			offset--;
			break;
		}

		if (next && get(next).deliver(iid)) {
			iid = 0;
		}

		break;
	}

	if (!prev) {
		return;
	}

	get(prev).update();
}

bool Conveyor::insert(uint iiid) {
	if (!iid) {
		iid = iiid;
		offset = steps/2;
		return true;
	}
	return false;
}

bool Conveyor::remove(uint iiid) {
	if (iid == iiid) {
		iid = 0;
		offset = 0;
		return true;
	}
	return false;
}

uint Conveyor::removeAny() {
	if (iid) {
		uint iiid = iid;
		iid = 0;
		offset = 0;
		return iiid;
	}
	return 0;
}

uint Conveyor::itemAt() {
	return iid;
}
