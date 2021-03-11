#pragma once

struct Explosion;

#include "slabmap.h"

struct Explosion {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Explosion,&Explosion::id> all;
	static Explosion& create(uint id);
	static Explosion& get(uint id);

	Health damage;
	float radius;
	float range;
	float rate;

	void destroy();
	void update();

	void define(Health damage, float radius, float rate);
};
