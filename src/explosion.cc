#include "common.h"
#include "entity.h"
#include "explosion.h"

void Explosion::reset() {
	all.clear();
}

void Explosion::tick() {
	for (auto& pair: all) {
		Explosion& explosion = pair.second;
		explosion.update();
	}
}

Explosion& Explosion::create(uint id) {
	Explosion& explosion = all[id];
	explosion.id = id;
	explosion.radius = 0;
	explosion.range = 0;
	explosion.rate = 0;
	explosion.damage = 0;
	return explosion;
}

Explosion& Explosion::get(uint id) {
	ensuref(all.count(id), "invalid explosion access %d", id);
	return all[id];
}

void Explosion::destroy() {
	all.erase(id);
}

void Explosion::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;

	if (radius >= range) {
		for (auto tid: Entity::intersecting(en.pos, range)) {
			if (tid == id) continue;
			Entity& te = Entity::get(tid);
			te.damage(damage);
		}
		en.remove();
		return;
	}

	radius += rate;
}

void Explosion::define(Health ddamage, float rradius, float rrate) {
	damage = ddamage;
	radius = 0.0;
	range = rradius;
	rate = rrate;
}