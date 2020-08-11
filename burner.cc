#include "common.h"
#include "entity.h"
#include "burner.h"

void Burner::reset() {
	all.clear();
}

Burner& Burner::create(uint id, uint sid) {
	Burner& burner = all.ref(id);
	burner.id = id;
	burner.energy = 0;
	burner.buffer = Energy::MJ(1);
	burner.store.burnerInit(id, sid, Mass::kg(3));
	return burner;
}

Burner& Burner::get(uint id) {
	ensuref(all.has(id), "invalid burner access %d", id);
	return all.ref(id);
}

void Burner::destroy() {
	store.burnerDestroy();
	all.drop(id);
}

Energy Burner::consume(Energy e) {
	if (e > energy) {
		Stack stack = store.removeFuel(store.fuelCategory, 1);
		if (!stack.iid) {
			Entity& en = Entity::get(id);
			if (en.spec->store) {
				stack = en.store().removeFuel(store.fuelCategory, 1);
				notef("%d", stack.iid);
			}
		}
		if (stack.iid && stack.size) {
			buffer = Item::get(stack.iid)->fuel.energy;
			energy += buffer;
		}
	}
	Energy c = energy < e ? energy: e;
	energy -= c;
	return c;
}
