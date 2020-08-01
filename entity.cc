#include "common.h"
#include "item.h"
#include "entity.h"
#include "chunk.h"
#include "sparse.h"
#include "json.hpp"

#include <map>
#include <stdio.h>
#include <fstream>
#include <algorithm>

void Entity::reset() {
	for (Entity& en: all) {
		en.destroy();
	}
	all.clear();
	grid.clear();
	Store::reset();
}

void Entity::preTick() {
	for (int id: removing) {
		Entity::get(id).destroy();
	}
	removing.clear();
}

uint Entity::next() {
	for (uint i = 0; i < MaxEntity; i++) {
		if (sequence == MaxEntity) {
			sequence = 0;
		}
		sequence++;
		if (!all.has(sequence)) {
			return sequence;
		}
	}
	fatalf("max entities reached");
}

Entity& Entity::create(uint id, Spec *spec) {
	Entity& en = all.ref(id);
	en.id = id;
	en.spec = spec;
	en.flags = GHOST;
	en.dir = Point::South();
	en.state = 0;

	if (spec->store) {
		Store::create(id);
	}

	if (spec->crafter) {
		Crafter::create(id);
	}

	if (spec->vehicle) {
		Vehicle::create(id);
	}

	if (spec->arm) {
		Arm::create(id);
	}

	if (spec->belt) {
		Belt::create(id);
	}

	if (spec->lift) {
		Lift::create(id);
	}

	en.pos = {0,0,0};
	en.move((Point){0,0,0});

	return en;
}

bool Entity::exists(uint id) {
	return id > 0 && all.has(id);
}

Entity& Entity::get(uint id) {
	ensuref(id > 0 && all.has(id), "invalid id %d", id);
	return all.ref(id);
}

bool Entity::fits(Spec *spec, Point pos, Point dir) {
	Box bounds = spec->box(pos, dir).shrink(0.1);
	switch (spec->place) {
		case Spec::Land: {
			if (!Chunk::isLand(bounds)) {
				return false;
			}
			break;
		}
		case Spec::Water: {
			if (!Chunk::isWater(bounds)) {
				return false;
			}
			break;
		}
		case Spec::Hill: {
			if (!Chunk::isHill(bounds)) {
				return false;
			}
			break;
		}
	}
	if (intersecting(bounds).size() > 0) {
		return false;
	}
	return true;
}

using json = nlohmann::json;

void Entity::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/entities.json");

	json state;
	state["sequence"] = sequence;
	out << state << "\n";

	for (Entity& en: all) {
		json state;
		state["id"] = en.id;
		state["spec"] = en.spec->name;
		state["flags"] = en.flags;
		state["pos"] = { en.pos.x, en.pos.y, en.pos.z };
		state["dir"] = { en.dir.x, en.dir.y, en.dir.z };

		out << state << "\n";
	}

	out.close();
}

void Entity::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/entities.json");

	std::string line;
	std::getline(in, line);
	auto state = json::parse(line);
	sequence = state["sequence"];

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Entity& en = create(state["id"], Spec::byName(state["spec"]));
		en.unindex();

		en.pos = (Point){state["pos"][0], state["pos"][1], state["pos"][2]};
		en.dir = (Point){state["dir"][0], state["dir"][1], state["dir"][2]};
		en.flags = state["flags"];

		en.index();
		en.materialize();
	}

	in.close();
}

std::unordered_set<uint> Entity::intersecting(Box box) {
	std::unordered_set<uint> hits;
	for (auto xy: Chunk::walk(box)) {
		for (uint id: grid[xy]) {
			if (get(id).box().intersects(box)) {
				hits.insert(id);
			}
		}
	}
	return hits;
}

uint Entity::at(Point p) {
	Box box = p.box();
	for (auto xy: Chunk::walk(box)) {
		for (uint id: grid[xy]) {
			if (get(id).box().intersects(box)) {
				return id;
			}
		}
	}
	return 0;
}

void Entity::destroy() {
	unindex();

	if (spec->store) {
		store().destroy();
	}

	if (spec->crafter) {
		crafter().destroy();
	}

	if (spec->vehicle) {
		vehicle().destroy();
	}

	if (spec->arm) {
		arm().destroy();
	}

	if (spec->belt) {
		belt().destroy();
	}

	if (spec->lift) {
		lift().destroy();
	}

	all.drop(id);
}

void Entity::remove() {
	removing.insert(id);
}

bool Entity::isGhost() {
	return (flags & GHOST) != 0;
}

Entity& Entity::setGhost(bool state) {
	flags = state ? (flags | GHOST) : (flags & ~GHOST);
	return *this;
}

bool Entity::isConstruction() {
	return (flags & CONSTRUCTION) != 0;
}

Entity& Entity::setConstruction(bool state) {
	flags = state ? (flags | CONSTRUCTION) : (flags & ~CONSTRUCTION);
	return *this;
}

bool Entity::isDeconstruction() {
	return (flags & DECONSTRUCTION) != 0;
}

Entity& Entity::setDeconstruction(bool state) {
	flags = state ? (flags | DECONSTRUCTION) : (flags & ~DECONSTRUCTION);
	return *this;
}

Box Entity::box() {
	return spec->box(pos, dir);
}

Entity& Entity::look(Point p) {
	unindex();
	dir = p.normalize();
	index();
	return *this;
}

Entity& Entity::lookAt(Point p) {
	look(p-pos);
	return *this;
}

bool Entity::lookAtPivot(Point o) {
	o = (o-pos).normalize();
	look(dir.pivot(o, 0.01));
	return dir == o;
}

Entity& Entity::index() {
	unindex();
	for (auto xy: Chunk::walk(box())) {
		grid[xy].insert(id);
	}
	return *this;
}

Entity& Entity::unindex() {
	for (auto xy: Chunk::walk(box())) {
		grid[xy].erase(id);
	}
	return *this;
}

Entity& Entity::manage() {
	if (!isGhost()) {
		if (spec->belt) {
			belt().manage();
		}
	}
	return *this;
}

Entity& Entity::unmanage() {
	if (!isGhost()) {
		if (spec->belt) {
			belt().unmanage();
		}
	}
	return *this;
}

Entity& Entity::construct() {
	setGhost(true);
	setConstruction(true);
	setDeconstruction(false);
	return *this;
}

Entity& Entity::deconstruct() {
	setGhost(true);
	setConstruction(false);
	setDeconstruction(true);
	unmanage();
	return *this;
}

Entity& Entity::materialize() {
	setGhost(false);
	setConstruction(false);
	setDeconstruction(false);
	manage();
	return *this;
}

Entity& Entity::move(Point p) {
	unmanage();
	unindex();
	pos = spec->aligned(p, dir);
	index();
	manage();
	return *this;
}

Entity& Entity::move(float x, float y, float z) {
	return move(Point(x, y, z));
}

Entity& Entity::floor(float level) {
	unmanage();
	unindex();
	pos.y = level + spec->h/2.0f;
	index();
	manage();
	return *this;
}

bool Entity::onFloor(float level) {
	float y = level + spec->h/2.0f;
	return std::abs(pos.y-y) < 0.01;
}

Entity& Entity::rotate() {
	if (spec->rotate) {
		unmanage();
		unindex();
		dir = dir.rotateHorizontal();
		pos = spec->aligned(pos, dir);
		index();
		manage();
	}
	return *this;
}

Entity& Entity::toggle() {
	if (spec->lift) {
		lift().toggle();
	}
	return *this;
}

Store& Entity::store() {
	return Store::get(id);
}

Crafter& Entity::crafter() {
	return Crafter::get(id);
}

Vehicle& Entity::vehicle() {
	return Vehicle::get(id);
}

Arm& Entity::arm() {
	return Arm::get(id);
}

Belt& Entity::belt() {
	return Belt::get(id);
}

Lift& Entity::lift() {
	return Lift::get(id);
}
