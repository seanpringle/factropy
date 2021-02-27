#include "common.h"
#include "sim.h"
#include "unveyor.h"

void Unveyor::reset() {
	all.clear();
}

void Unveyor::tick() {
	for (auto& unveyor: all) {
		unveyor.update();
	}
}

Unveyor& Unveyor::create(uint id) {
	Entity& en = Entity::get(id);
	Unveyor& unveyor = all[id];
	unveyor.id = id;
	unveyor.partner = 0;
	unveyor.entry = en.spec->unveyorEntry;
	return unveyor;
}

Unveyor& Unveyor::get(uint id) {
	return all.refer(id);
}

void Unveyor::destroy() {
	all.erase(id);
}

Box Unveyor::range() {
	Entity& en = Entity::get(id);
	Point dir = entry ? en.dir: -en.dir;
	Point far = (dir * 10.0f) + en.pos;
	return Box(en.pos, far).grow(0.1f);
}

void Unveyor::manage() {
	if (!partner) {
		float dist = 0.0f;
		Entity& en = Entity::get(id);

		for (uint pid: Entity::intersecting(range())) {
			if (id == pid) continue;
			if (!all.has(pid)) continue;
			Unveyor& other = get(pid);
			if (other.partner) continue;
			Entity& eo = Entity::get(pid);
			if (!(en.dir == eo.dir && entry != other.entry)) continue;
			float d = en.pos.distance(eo.pos);
			if (!partner || d < dist) {
				partner = pid;
				dist = d;
			}
		}

		if (partner) {
			get(partner).partner = id;
			auto& send = get(entry ? id: partner);
			send.items.clear();
			send.items.resize((uint)dist);
		}
	}
}

void Unveyor::unmanage() {
	if (partner) {
		Unveyor& other = get(partner);
		ensure(other.partner == id);
		other.partner = 0;
		partner = 0;
	}
}

void Unveyor::update() {
	if (!partner || !entry) return;

	auto& send = Conveyor::get(id);
	auto& recv = Conveyor::get(partner);

	item* prev = nullptr;
	for (uint slot = 0; slot < items.size(); slot++) {
		items[slot].update(prev, recv, send.steps);
		prev = &items[slot];
	}

	if (send.iid && send.offset == send.steps/2 && prev->deliver(send.iid, send.steps)) {
		send.remove(send.iid);
	}
}

void Unveyor::item::update(Unveyor::item* prev, Conveyor& recv, uint steps) {

	if (!iid) {
		return;
	}

	if (prev && prev->offset && offset <= prev->offset) {
		return;
	}

	if (offset > 0) {
		offset--;
		return;
	}

	if (prev && prev->deliver(iid, steps)) {
		iid = 0;
		return;
	}

	if (!prev && recv.deliver(iid)) {
		iid = 0;
		return;
	}
}

bool Unveyor::item::deliver(uint iiid, uint steps) {
	if (!iid) {
		iid = iiid;
		offset = steps-1;
		return true;
	}
	return false;
}
