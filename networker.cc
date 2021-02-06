#include "common.h"
#include "networker.h"

void Networker::reset() {
	all.clear();
}

void Networker::tick() {
	for (auto& pair: all) {
		pair.second.update();
	}
}

Networker& Networker::create(uint id) {
	ensuref(!all.count(id), "double-create networker %d", id);
	Networker& networker = all[id];
	networker.id = id;
	return networker;
}

Networker& Networker::get(uint id) {
	ensuref(all.count(id) > 0, "invalid networker access %d", id);
	return all[id];
}

void Networker::destroy() {
	all.erase(id);
}

void Networker::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;
}
