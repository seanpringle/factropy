#pragma once

// Drone components are deployed by Depots to move items between Stores.

struct Drone;

#include "item.h"
#include "slabmap.h"

struct Drone {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Drone,&Drone::id> all;
	static Drone& create(uint id);
	static Drone& get(uint id);

	enum Stage {
		ToSrc = 1,
		ToDst,
		ToDep,
		Stranded,
	};

	uint iid;
	uint dep;
	uint src;
	uint dst;
	Stack stack;
	bool srcGhost;
	bool dstGhost;
	enum Stage stage;

	void destroy();
	void update();
	bool travel(uint eid);
};
