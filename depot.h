#ifndef _H_depot
#define _H_depot

struct Depot;

#include "slabmap.h"
#include "entity.h"

struct Depot {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Depot,uint,&Depot::id> all;
	static Depot& create(uint id);
	static Depot& get(uint id);

	uint64_t pause;
	std::set<uint> drones;

	void destroy();
	void update();
	void dispatch(uint dep, uint src, uint dst, Stack stack);
};

#endif