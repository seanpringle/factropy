#include "common.h"
#include "entity.h"
#include "burner.h"

void Burner::reset() {
	all.clear();
}

Burner& Burner::create(uint id) {
	Burner& burner = all.ref(id);
	burner.id = id;
	burner.energy = 0;
	burner.buffer = Energy::MJ(1);
	burner.store.burnerInit(id, Mass::kg(3));
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
		Stack stack = store.removeFuel(1);
		if (stack.iid && stack.size) {
			buffer = Item::get(stack.iid)->fuel.energy;
			energy += buffer;
		}
	}
	Energy c = energy < e ? energy: e;
	energy -= c;
	return c;
}
