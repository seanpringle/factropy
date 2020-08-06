#ifndef _H_depot
#define _H_depot

struct Depot;

#include "sparse.h"
#include "entity.h"
#include <map>

struct Depot {
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline std::map<uint,Depot> all;
	static Depot& create(uint id);
	static Depot& get(uint id);

	uint id;
	uint64_t pause;
	std::set<uint> drones;

	void destroy();
	void update();
	void dispatch(uint dep, uint src, uint dst, Stack stack);
};

#endif