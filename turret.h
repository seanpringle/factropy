#ifndef _H_turret
#define _H_turret

struct Turret;

#include "sim.h"

struct Turret {
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline std::map<uint,Turret> all;
	static Turret& create(uint id);
	static Turret& get(uint id);

	uint id;
	uint tid; // target
	Point aim;
	int cool;

	void destroy();
	void update();

	bool aimAt(Point o);
};

#endif