#include "common.h"
#include "computer.h"

void Computer::reset() {
	all.clear();
}

void Computer::tick() {
	for (auto& pair: all) {
		pair.second.update();
	}
}

Computer& Computer::create(uint id) {
	ensuref(!all.count(id), "double-create computer %d", id);
	Computer& computer = all[id];
	computer.id = id;
	return computer;
}

Computer& Computer::get(uint id) {
	ensuref(all.count(id) > 0, "invalid computer access %d", id);
	return all[id];
}

void Computer::destroy() {
	all.erase(id);
}

void Computer::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;
}
