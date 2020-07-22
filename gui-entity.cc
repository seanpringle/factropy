#include "common.h"
#include "entity.h"

GuiEntity::GuiEntity() {
	id = 0;
	spec = NULL;
	dir = South;
	pos = {0,0,0};
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

GuiFakeEntity* GuiFakeEntity::move(float x, float y, float z) {
	return move(Point(x, y, z));
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
