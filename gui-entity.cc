#include "common.h"
#include "entity.h"

GuiEntity::GuiEntity() {
	id = 0;
	spec = NULL;
	pos = {0,0,0};
	dir = Point::South;
	state = 0;
	ghost = false;
	transform = Mat4::identity;

	burner.energy = 0;
	burner.buffer = 0;

	store.limit = 0;
	store.usage = 0;

	crafter.recipe = NULL;
	crafter.progress = 0.0f;
	crafter.inputsProgress = 0.0f;
}

GuiEntity::GuiEntity(uint id) : GuiEntity() {
	Entity& en = Entity::get(id);
	this->id = id;
	spec = en.spec;
	pos = en.pos;
	dir = en.dir;
	state = en.state;
	ghost = en.isGhost();
	updateTransform();

	if (spec->consumeChemical) {
		burner.energy = en.burner().energy;
		burner.buffer = en.burner().buffer;
	}

	if (spec->store) {
		store.limit = en.store().limit();
		store.usage = en.store().usage();
	}

	if (spec->crafter) {
		crafter.recipe = en.crafter().recipe;
		crafter.progress = en.crafter().progress;
		crafter.inputsProgress = en.crafter().inputsProgress();
		crafter.completed = en.crafter().completed;
	}

	if (spec->pipe) {
		pipe.fid = en.pipe().network ? en.pipe().network->fid: 0;
		pipe.level = pipe.fid ? en.pipe().network->level(): 0.0f;
	}
}

GuiEntity::~GuiEntity() {
}

Box GuiEntity::box() {
	return spec->box(pos, dir);
}

Box GuiEntity::miningBox() {
	return box().grow(0.5f);
}

Point GuiEntity::ground() {
	return {pos.x, pos.y - spec->collision.h/2.0f, pos.z};
}

void GuiEntity::updateTransform() {
	Mat4 r = dir.rotation();
	Mat4 t = Mat4::translate(pos.x, pos.y, pos.z);
	transform = r * t;
}

// GuiFakeEntity

GuiFakeEntity::GuiFakeEntity(Spec* spec) : GuiEntity() {
	id = 0;
	this->spec = spec;
	dir = Point::South;
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
	pos.y = level + spec->collision.h/2.0f;
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
