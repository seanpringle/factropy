#include "common.h"
#include "entity.h"

GuiEntity::GuiEntity() {
	id = 0;
	spec = NULL;
	pos = {0,0,0};
	dir = Point::South();
	transform = MatrixIdentity();
}

GuiEntity::GuiEntity(uint id) {
	Entity& en = Entity::get(id);
	this->id = id;
	spec = en.spec;
	pos = en.pos;
	dir = en.dir;
	state = en.state;
	ghost = en.isGhost();
	updateTransform();
}

GuiEntity::~GuiEntity() {
}

Box GuiEntity::box() {
	return spec->box(pos, dir);
}

void GuiEntity::updateTransform() {
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
	transform = MatrixMultiply(r, t);
}

// GuiFakeEntity

GuiFakeEntity::GuiFakeEntity(Spec* spec) : GuiEntity() {
	id = 0;
	this->spec = spec;
	dir = Point::South();
	state = 0;
	ghost = true;
	move((Point){0,0,0});
}

GuiFakeEntity::~GuiFakeEntity() {
}

GuiFakeEntity* GuiFakeEntity::move(Point p) {
	pos = spec->aligned(p, dir);
	updateTransform();
	return this;
}

GuiFakeEntity* GuiFakeEntity::move(float x, float y, float z) {
	return move(Point(x, y, z));
}

GuiFakeEntity* GuiFakeEntity::floor(float level) {
	pos.y = level + spec->h/2.0f;
	updateTransform();
	return this;
}

GuiFakeEntity* GuiFakeEntity::rotate() {
	if (spec->rotate) {
		dir = dir.rotateHorizontal();
		pos = spec->aligned(pos, dir);
		updateTransform();
	}
	return this;
}
