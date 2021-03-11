#include "common.h"
#include "sim.h"
#include "entity.h"
#include "missile.h"

void Missile::reset() {
	all.clear();
}

void Missile::tick() {
	for (auto& missile: all) {
		missile.update();
	}
}

Missile& Missile::create(uint id) {
	Missile& missile = all[id];
	missile.id = id;
	missile.tid = 0;
	missile.attacking = false;
	missile.aim = Point::Zero;
	return missile;
}

Missile& Missile::get(uint id) {
	return all.refer(id);
}

void Missile::destroy() {
	all.erase(id);
}

void Missile::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;
	if (!attacking) return;

	bool impact = false;
	Point next = aim;

	if (en.spec->missileBallistic) {
		Point dir = aim - en.pos;

		next = en.pos + dir.normalize() * en.spec->missileSpeed;
		impact = false;

		if (en.pos.distance(aim) < en.spec->missileSpeed*1.1) {
			next = aim;
			impact = true;
		}
	}

	if (!en.spec->missileBallistic) {
		if (tid && !en.spec->missileBallistic && Entity::exists(tid)) {
			aim = Entity::get(tid).pos;
		}

		Point dir = aim - en.pos;

		next = en.pos + dir.normalize() * en.spec->missileSpeed;
		impact = false;

		if (en.pos.distance(aim) < en.spec->missileSpeed*1.1) {
			next = aim;
			impact = true;
		}

		if (!impact) {
			impact = Sim::rayCast(en.pos, next, 0.25, [&](uint cid) { return cid != id; });
		}
	}

	en.lookAt(next);
	en.move(next);

	if (impact) en.explode();
}

void Missile::attack(uint aid) {
	attacking = true;
	tid = aid;
	aim = Entity::get(tid).pos;
}

