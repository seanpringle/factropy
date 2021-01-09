#include "common.h"
#include "entity.h"
#include "turret.h"
#include "missile.h"

void Turret::reset() {
	all.clear();
}

void Turret::tick() {
	for (auto& pair: all) {
		Turret& turret = pair.second;
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
	ensuref(all.count(id), "invalid turret access %d", id);
	return all[id];
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
		for (auto rid: Entity::enemiesInRange(en.pos, en.spec->turretRange)) {
			Entity& te = Entity::get(rid);
			if (Sim::rayCast(en.pos, te.pos, 0.25, [&](uint cid) { return cid != id && cid != rid; })) {
				tid = rid;
				break;
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
	aim = aim.pivot(p, 0.03);
	return aim == p;
}

