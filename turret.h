#ifndef _H_turret
#define _H_turret

struct Turret;

#include "sim.h"
#include "slabmap.h"

struct Turret {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Turret,&Turret::id> all;
	static Turret& create(uint id);
	static Turret& get(uint id);

	uint tid; // target
	Point aim;
	int cool;

	void destroy();
	void update();

	bool aimAt(Point o);
};

#endif