#include "common.h"
#include "entity.h"
#include "sparse.h"
#include "json.hpp"

#include <map>
#include <stdio.h>
#include <filesystem>
#include <fstream>

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
		dirs.set(id, South);
	}

	en.move((Point){0,0,0});

	return en;
}

Entity& Entity::get(int id) {
	ensuref(all.has(id), "invalid id %d", id);
	return all.ref(id);
}

namespace fs = std::filesystem;
using json = nlohmann::json;

void Entity::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/entities.json");

	for (auto &en: all) {
		json state;
		state["id"] = en.id;
		state["spec"] = en.spec->name;
		state["flags"] = en.flags;
		state["pos"] = { en.pos.x, en.pos.y, en.pos.z };

		if (en.spec->hasDirection()) {
			state["dir"] = dirs.get(en.id);
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
		en.pos = (Point){state["pos"][0], state["pos"][1], state["pos"][2]};
		en.flags = state["flags"];

		if (en.spec->hasDirection()) {
			dirs.set(en.id, state["dir"]);
		}
	}

	in.close();
}

void Entity::reset() {
	for (auto& en: all) {
		en.destroy();
	}
}

void Entity::destroy() {
	if (spec->hasDirection()) {
		dirs.drop(id);
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
	return (spec->hasDirection()) ? dirs.get(id): South;
}

Entity& Entity::face(enum Direction d) {
	if (spec->rotate || (isGhost() && spec->rotateGhost)) {
		dirs.set(id, d);
	}
	return *this;
}

Entity& Entity::move(Point p) {
	pos = spec->aligned(p, dir());
	return *this;
}

Entity& Entity::floor(float level) {
	auto animation = &spec->animations[dir()];
	pos.y = level + animation->h/2.0f;
	return *this;
}

Entity& Entity::rotate() {
	if (spec->rotate || (isGhost() && spec->rotateGhost)) {
		dirs.set(id, Directions::rotate(dirs.get(id)));
		pos = spec->aligned(pos, dir());
	}
	return *this;
}

// GuiEntity

GuiEntity::GuiEntity() {
	id = 0;
	spec = NULL;
	dir = South;
	pos = {0};
}

GuiEntity::GuiEntity(int id) {
	Entity& en = Entity::get(id);
	this->id = id;
	spec = en.spec;
	pos = en.pos;
	dir = en.dir();
	ghost = en.isGhost();
}

GuiEntity::~GuiEntity() {
}

Box GuiEntity::box() {
	auto animation = &spec->animations[dir];
	return (Box){pos.x, pos.y, pos.z, animation->w, animation->h, animation->d};
}

Matrix GuiEntity::transform() {
	Matrix r = MatrixRotateY(Directions::degrees(dir)*DEG2RAD);
	Matrix t = MatrixTranslate(pos.x, pos.y, pos.z);
	return MatrixMultiply(r, t);
}

// GuiFakeEntity

GuiFakeEntity::GuiFakeEntity(Spec* spec) : GuiEntity() {
	id = 0;
	this->spec = spec;
	dir = South;
	ghost = true;
	move((Point){0,0,0});
}

GuiFakeEntity::~GuiFakeEntity() {
}

GuiFakeEntity* GuiFakeEntity::face(enum Direction d) {
	dir = (spec->rotate || (ghost && spec->rotateGhost)) ? d: South;
	return this;
}

GuiFakeEntity* GuiFakeEntity::move(Point p) {
	pos = spec->aligned(p, dir);
	return this;
}

GuiFakeEntity* GuiFakeEntity::floor(float level) {
	auto animation = &spec->animations[dir];
	pos.y = level + animation->h/2.0f;
	return this;
}

GuiFakeEntity* GuiFakeEntity::rotate() {
	if (spec->rotate || (ghost && spec->rotateGhost)) {
		dir = Directions::rotate(dir);
		pos = spec->aligned(pos, dir);
	}
	return this;
}
