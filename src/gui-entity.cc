#include "common.h"
#include "entity.h"

GuiEntity::GuiEntity() {
	id = 0;
	spec = NULL;
	pos = {0,0,0};
	dir = Point::South;
	aim = Point::South;
	state = 0;
	health = 1.0;
	ghost = false;
	transform = Mat4::identity;

	burner.energy = 0;
	burner.buffer = 0;

	store.limit = 0;
	store.usage = 0;

	crafter.recipe = NULL;
	crafter.progress = 0.0f;
	crafter.inputsProgress = 0.0f;

	pipe.fid = 0;
	pipe.level = 0;

	explosion.radius = 0;
}

GuiEntity::GuiEntity(uint id) : GuiEntity() {
	Entity& en = Entity::get(id);
	this->id = id;
	spec = en.spec;
	pos = en.pos;
	dir = en.dir;
	aim = Point::South;
	state = en.state;
	health = en.health;
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

	if (spec->explosion) {
		explosion.radius = en.explosion().radius;
	}

	if (spec->turret) {
		aim = en.turret().aim;
	}

	if (spec->ropeway) {
		auto& r = en.ropeway();
		aim = r.aim;
		ropeway.next = r.next;
		ropeway.a = r.arrive();
		ropeway.c = r.depart();
		if (r.next) {
			auto &s = Entity::get(r.next).ropeway();
			ropeway.b = s.arrive();
			ropeway.d = s.depart();
		} else {
			ropeway.b = Point::Zero;
			ropeway.d = Point::Zero;
		}
	}

	if (spec->conveyor) {
		conveyor.iid = en.conveyor().iid;
		conveyor.offset = en.conveyor().offset;
	}

	if (spec->arm) {
		arm.iid = en.arm().iid;
	}

	if (spec->drone) {
		drone.iid = en.drone().iid;
	}
}

GuiEntity::~GuiEntity() {
}

Box GuiEntity::box() {
	return spec->box(pos, dir, spec->collision);
}

Box GuiEntity::selectionBox() {
	return spec->box(pos, dir, spec->selection);
}

Box GuiEntity::southBox() {
	return spec->southBox(pos);
}

Box GuiEntity::miningBox() {
	return box().grow(0.5f);
}

Point GuiEntity::ground() {
	return {pos.x, pos.y - spec->collision.h/2.0f, pos.z};
}

Sphere GuiEntity::sphere() {
	return box().sphere();
}

void GuiEntity::updateTransform() {
	Mat4 r = dir.rotation();
	Mat4 t = Mat4::translate(pos.x, pos.y, pos.z);
	transform = r * t;
}

Mat4 GuiEntity::partTransform(Part* part) {
	if (part->pivot) {
		Mat4 r = aim.rotation();
		Mat4 t = Mat4::translate(pos.x, pos.y, pos.z);
		return r * t;
	}
	return transform;
}

bool GuiEntity::connectable(GuiEntity* other) {
	return spec->ropeway && other->spec->ropeway && pos.distance(other->pos) < 50.0f;
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

GuiFakeEntity* GuiFakeEntity::getConfig(Entity& en) {
	if (en.spec != spec) return this;

	if (en.spec->crafter) {
		crafter.recipe = en.crafter().recipe;
	}

	if (en.spec->logistic && en.spec->store) {
		for (auto level: en.store().levels) {
			store.levels.push_back(level);
		}
	}
	return this;
}

GuiFakeEntity* GuiFakeEntity::setConfig(Entity& en) {
	if (en.spec != spec) return this;

	if (en.spec->crafter) {
		en.crafter().nextRecipe = crafter.recipe;
	}

	if (en.spec->logistic && en.spec->store) {
		for (auto level: store.levels) {
			en.store().levelSet(level.iid, level.lower, level.upper);
		}
	}
	return this;
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
