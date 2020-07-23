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
	directions.clear();
	grid.clear();
	Store::reset();
}

int Entity::next() {
	int i = all.next(false);
	ensuref(i > 0, "max entities reached");
	return i;
}

Entity& Entity::create(int id, Spec *spec) {
	Entity& en = all.ref(id);
	en.id = id;
	en.spec = spec;
	en.flags = 0;

	if (spec->hasDirection()) {
		directions.set(id, South);
	}

	if (spec->hasOrientation()) {
		orientations.set(id, Point(0,0,1));
	}

	if (spec->hasStore()) {
		Store::create(id);
	}

	if (spec->vehicle) {
		Vehicle::create(id);
	}

	en.pos = {0,0,0};
	en.move((Point){0,0,0});

	return en;
}

bool Entity::exists(int id) {
	return all.has(id);
}

Entity& Entity::get(int id) {
	ensuref(all.has(id), "invalid id %d", id);
	return all.ref(id);
}

using json = nlohmann::json;

void Entity::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/entities.json");

	for (Entity& en: all) {
		json state;
		state["id"] = en.id;
		state["spec"] = en.spec->name;
		state["flags"] = en.flags;
		state["pos"] = { en.pos.x, en.pos.y, en.pos.z };

		if (en.spec->hasDirection()) {
			state["dir"] = directions.get(en.id);
		}

		out << state << "\n";
	}

	out.close();
}

void Entity::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/entities.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Entity& en = create(state["id"], Spec::byName(state["spec"]));
		en.unindex();

		en.pos = (Point){state["pos"][0], state["pos"][1], state["pos"][2]};
		en.flags = state["flags"];

		if (en.spec->hasDirection()) {
			directions.set(en.id, state["dir"]);
		}

		en.index();
	}

	in.close();
}

std::unordered_set<int> Entity::intersecting(Box box) {
	std::unordered_set<int> hits;
	for (auto xy: Chunk::walk(box)) {
		for (int id: grid[xy]) {
			if (get(id).box().intersects(box)) {
				hits.insert(id);
			}
		}
	}
	return hits;
}

void Entity::destroy() {
	unindex();
	if (spec->hasDirection()) {
		directions.drop(id);
	}
	if (spec->hasOrientation()) {
		orientations.drop(id);
	}
	if (spec->hasStore()) {
		store().destroy();
	}
	if (spec->vehicle) {
		vehicle().destroy();
	}
	all.drop(id);
}

bool Entity::isGhost() {
	return (flags & GHOST) != 0;
}

Entity& Entity::setGhost(bool state) {
	flags = state ? (flags | GHOST) : (flags & ~GHOST);
	return *this;
}

Box Entity::box() {
	auto animation = &spec->animations[dir()];
	return (Box){pos.x, pos.y, pos.z, animation->w, animation->h, animation->d};
}

enum Direction Entity::dir() {
	return (spec->hasDirection()) ? directions.get(id): South;
}

Point Entity::looking() {
	return (spec->hasOrientation()) ? orientations.get(id): Point(0,0,-1);
}

void Entity::look(Point p) {
	if (spec->hasOrientation()) {
		unindex();
		orientations.set(id, p.normalize());
		index();
	}
}

void Entity::lookAt(Point p) {
	look(p-pos);
}

bool Entity::lookAtPivot(Point o) {
	o = (o-pos).normalize();
	look(looking().pivot(o, 0.01));
	return looking() == o;
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

Entity& Entity::face(enum Direction d) {
	if (spec->rotate || (isGhost() && spec->rotateGhost)) {
		unindex();
		directions.set(id, d);
		index();
	}
	return *this;
}

Entity& Entity::move(Point p) {
	unindex();
	pos = spec->aligned(p, dir());
	index();
	return *this;
}

Entity& Entity::move(float x, float y, float z) {
	return move(Point(x, y, z));
}

Entity& Entity::floor(float level) {
	unindex();
	auto animation = &spec->animations[dir()];
	pos.y = level + animation->h/2.0f;
	index();
	return *this;
}

Entity& Entity::rotate() {
	if (spec->rotate || (isGhost() && spec->rotateGhost)) {
		unindex();
		directions.set(id, Directions::rotate(directions.get(id)));
		pos = spec->aligned(pos, dir());
		index();
	}
	return *this;
}

Store& Entity::store() {
	return Store::get(id);
}

Vehicle& Entity::vehicle() {
	return Vehicle::get(id);
}
