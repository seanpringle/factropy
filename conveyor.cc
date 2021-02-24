#include "common.h"
#include "sim.h"
#include "conveyor.h"

void Conveyor::reset() {
	all.clear();
}

void Conveyor::tick() {
	if (rebuild) {
		rebuild = false;

		leadersStraight.clear();
		leadersCircular.clear();
		for (auto& conveyor: all) {
			conveyor.marked = false;
			conveyor.managed = !Entity::get(conveyor.id).isGhost();
		}

		// identify belt leaders
		for (auto& leader: all) {
			if (!leader.managed) continue;

			if (!leader.next) {
				leadersStraight.push_back(leader.id);
				leader.marked = true;
				uint prev = leader.prev;
				while (prev) {
					Conveyor& before = get(prev);
					ensure(!before.marked);
					before.marked = true;
					prev = before.prev;
				}
			}
		}

		for (auto& leader: all) {
			if (!leader.managed) continue;

			if (!leader.marked) {
				ensure(leader.next && leader.prev);
				leadersCircular.push_back(leader.id);
				leader.marked = true;
				uint prev = leader.prev;
				while (prev) {
					Conveyor& before = get(prev);
					ensure(before.next && before.prev);
					if (before.marked) break;
					before.marked = true;
					prev = before.prev;
				}
			}
		}

		for (auto id: leadersStraight) {
			Conveyor& leader = get(id);
			leader.cnext = nullptr;
			leader.cprev = leader.prev ? &all[leader.prev]: nullptr;
			uint prev = leader.prev;
			while (prev) {
				Conveyor& before = get(prev);
				before.cnext = before.next ? &all[before.next]: nullptr;
				before.cprev = before.prev ? &all[before.prev]: nullptr;
				prev = before.prev;
			}
		}

		for (auto id: leadersCircular) {
			Conveyor& leader = get(id);
			leader.cnext = nullptr;
			leader.cprev = leader.prev ? &all[leader.prev]: nullptr;
			uint prev = leader.prev;
			while (prev && prev != leader.id) {
				Conveyor& before = get(prev);
				before.cnext = before.next ? &all[before.next]: nullptr;
				before.cprev = before.prev ? &all[before.prev]: nullptr;
				prev = before.prev;
			}
			get(leader.next).cprev = nullptr;
		}
	}

	for (auto id: leadersStraight) {
		Conveyor& leader = get(id);
		leader.update();
	}

	for (auto id: leadersCircular) {
		Conveyor& leader = get(id);
		if (leader.iid && leader.offset == 0) {
			uint iid = leader.iid;
			leader.iid = 0;
			leader.update();
			get(leader.next).deliver(iid);
			continue;
		}
		if (leader.iid && leader.offset > 0) {
			uint iid = leader.iid;
			uint offset = leader.offset;
			leader.iid = 0;
			leader.offset = 0;
			leader.update();
			leader.iid = iid;
			leader.offset = offset-1;
			continue;
		}
		leader.update();
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
	conveyor.cnext = nullptr;
	conveyor.cprev = nullptr;
	conveyor.marked = false;
	conveyor.managed = false;
	return conveyor;
}

Conveyor& Conveyor::get(uint id) {
	ensuref(all.count(id), "invalid conveyor access %d", id);
	return all[id];
}

void Conveyor::destroy() {
	ensuref(!managed, "attempt to destroy managed conveyor %d", id);
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

	rebuild = true;
	managed = true;

	Entity& en = Entity::get(id);

	Box ib = input().box().grow(0.1f);
	Box ob = output().box().grow(0.1f);

	for (auto oid: Entity::intersecting(ob)) {
		if (oid == id) continue;
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
		if (oid == id) continue;
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

	rebuild = true;
	managed = false;

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
	if (!iid) {
		iid = iiid;
		offset = steps-1;
		return true;
	}
	return false;
}

void Conveyor::update() {
	for(;;) {
		if (!iid) {
			break;
		}

		uint nextOffset = cnext ? cnext->offset: 0;

		if (cnext && nextOffset && offset <= nextOffset) {
			break;
		}

		if (!cnext && offset <= steps/2) {
			break;
		}

		if (offset > 0) {
			offset--;
			break;
		}

		if (cnext && cnext->deliver(iid)) {
			iid = 0;
		}

		break;
	}

	if (!cprev) {
		return;
	}

	cprev->update();
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
