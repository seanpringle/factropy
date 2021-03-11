#pragma once

// Missile components move between points and trigger an Explosion.

struct Missile;

#include "slabmap.h"

struct Missile {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Missile,&Missile::id> all;
	static Missile& create(uint id);
	static Missile& get(uint id);

	uint tid;
	Point aim;
	bool attacking;

	void destroy();
	void update();
	void attack(uint tid);
};
