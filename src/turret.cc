#include "common.h"
#include "entity.h"
#include "turret.h"
#include "missile.h"

void Turret::reset() {
	all.clear();
}

void Turret::tick() {
	for (auto& turret: all) {
		turret.update();
	}
}

Turret& Turret::create(uint id) {
	Turret& turret = all[id];
	turret.id = id;
	turret.tid = 0;
	turret.aim = Point::South;
	turret.cool = 0;
	return turret;
}

Turret& Turret::get(uint id) {
	return all.refer(id);
}

void Turret::destroy() {
	all.erase(id);
}

void Turret::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;

	if (cool) {
		cool--;
	}

	if (!tid) {
		float tdist = 0.0f;
		for (auto rid: Entity::enemiesInRange(en.pos, en.spec->turretRange)) {
			Entity& ren = Entity::get(rid);
			if (tid && ren.pos.distance(en.pos) > tdist) continue;
			if (Sim::rayCast(en.pos, ren.pos, 0.25, [&](uint cid) { return cid != id && cid != rid; })) {
				tid = rid;
				tdist = ren.pos.distance(en.pos);
			}
		}
	}

	if (!tid || !Entity::exists(tid)) {
		tid = 0;
		return;
	}

	Entity& te = Entity::get(tid);

	if (!aimAt(te.pos)) {
		return;
	}

	if (!cool) {
		Entity& be = Entity::create(Entity::next(), Spec::byName(en.spec->turretBulletSpec));
		be.missile().attack(tid);
		be.move(en.pos + (te.pos-en.pos).normalize());
		be.materialize();
		cool = en.spec->turretCooldown;
	}
}

bool Turret::aimAt(Point p) {
	Entity& en = Entity::get(id);
	p = (p-en.pos).normalize();
	aim = aim.pivot(p, en.spec->turretPivot);
	return aim == p;
}

