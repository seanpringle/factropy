#include "common.h"
#include "entity.h"

GuiEntity::GuiEntity() {
	id = 0;
	spec = NULL;
	pos = {0,0,0};
	dir = Point::South();
}

GuiEntity::GuiEntity(int id) {
	Entity& en = Entity::get(id);
	this->id = id;
	spec = en.spec;
	pos = en.pos;
	dir = en.dir;
	ghost = en.isGhost();
}

GuiEntity::~GuiEntity() {
}

Box GuiEntity::box() {
	return spec->box(pos, dir);
}

Matrix GuiEntity::transform() {
	Matrix r = MatrixIdentity();

	// https://gamedev.stackexchange.com/questions/15070/orienting-a-model-to-face-a-target
	if (dir == Point::North()) {
		r = MatrixRotate(Point::Up(), 180.0f*DEG2RAD);
	}
	else
	if (dir == Point::South()) {
		r = MatrixIdentity();
	}
	else {
		Point axis = Point::South().cross(dir);
		float angle = std::acos(Point::South().dot(dir));
		r = MatrixRotate(axis, angle);
	}

	Matrix t = MatrixTranslate(pos.x, pos.y, pos.z);
	return MatrixMultiply(r, t);
}

// GuiFakeEntity

GuiFakeEntity::GuiFakeEntity(Spec* spec) : GuiEntity() {
	id = 0;
	this->spec = spec;
	dir = Point::South();
	ghost = true;
	move((Point){0,0,0});
}

GuiFakeEntity::~GuiFakeEntity() {
}

GuiFakeEntity* GuiFakeEntity::move(Point p) {
	pos = spec->aligned(p, dir);
	return this;
}

GuiFakeEntity* GuiFakeEntity::move(float x, float y, float z) {
	return move(Point(x, y, z));
}

GuiFakeEntity* GuiFakeEntity::floor(float level) {
	pos.y = level + spec->h/2.0f;
	return this;
}

GuiFakeEntity* GuiFakeEntity::rotate() {
	if (spec->align) {
		dir = dir.rotateHorizontal();
		pos = spec->aligned(pos, dir);
	}
	return this;
}
