#include "common.h"
#include "belt.h"

void Belt::reset() {
	all.clear();
}

void Belt::tick() {
}

Belt& Belt::create(uint id) {
	Belt& belt = all.ref(id);
	belt.id = id;
	belt.segment = NULL;
	return belt;
}

Belt& Belt::get(uint id) {
	ensuref(all.has(id), "invalid belt access %d", id);
	return all.ref(id);
}

void Belt::destroy() {
	all.drop(id);
}

Belt& Belt::manage() {
	return *this;
}

Belt& Belt::unmanage() {
	return *this;
}

BeltSegment::BeltSegment() {
	all.insert(this);
}

BeltSegment::~BeltSegment() {
	all.erase(this);
	for (Belt* belt: belts) {
		belt->segment = NULL;
	}
	belts.clear();
}