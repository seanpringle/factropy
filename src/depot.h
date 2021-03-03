#pragma once

// Depot components deploy Drones to construct and deconstruct Ghosts, and supply
// Stores within their vicinity. Depots don't form networks or send drones over
// long distances, but they do co-operate within shared areas to load-balance
// drone activity.

struct Depot;

#include "slabmap.h"
#include "miniset.h"
#include "entity.h"

struct Depot {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Depot,&Depot::id> all;
	static Depot& create(uint id);
	static Depot& get(uint id);

	uint64_t pause;
	miniset<uint> drones;

	void destroy();
	void update();
	void dispatch(uint dep, uint src, uint dst, Stack stack);
};
