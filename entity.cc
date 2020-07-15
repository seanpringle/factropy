#include "common.h"
#include "entity.h"
#include "sparse.h"

#include <map>
#include <stdio.h>

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

Entity& Entity::load(int id) {
	ensuref(all.has(id), "invalid id %d", id);
	return all.ref(id);
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

GuiEntity::GuiEntity(int id) {
	Entity& en = Entity::load(id);
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

GuiFakeEntity::GuiFakeEntity(Spec* spec) {
	this->spec = spec;
	dir = South;
	ghost = true;
	move((Point){0,0,0});
}

GuiFakeEntity::~GuiFakeEntity() {
}

Box GuiFakeEntity::box() {
	auto animation = &spec->animations[dir];
	return (Box){pos.x, pos.y, pos.z, animation->w, animation->h, animation->d};
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
