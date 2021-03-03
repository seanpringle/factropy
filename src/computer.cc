#include "common.h"
#include "computer.h"
#include "entity.h"

void Computer::reset() {
	all.clear();
}

void Computer::tick() {
	for (auto& computer: all) {
		computer.update();
	}
}

Computer& Computer::create(uint id) {
	ensuref(!all.count(id), "double-create computer %d", id);
	Computer& computer = all[id];
	computer.id = id;
	return computer;
}

Computer& Computer::get(uint id) {
	return all.refer(id);
}

void Computer::destroy() {
	all.erase(id);
}

void Computer::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;
}
