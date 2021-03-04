#pragma once

// Unveyor (underground conveyor) components link up with Conveyors to allow
// short distance underground belts.

struct Unveyor;

#include "entity.h"
#include "conveyor.h"
#include "slabmap.h"

struct Unveyor {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Unveyor,&Unveyor::id> all;
	static Unveyor& create(uint id);
	static Unveyor& get(uint id);

	// items are a mini invisible conveyor system
	struct item {
		uint offset = 0;
		uint iid = 0;
		void update(Unveyor::item* prev, Conveyor& recv, uint steps);
		bool deliver(uint iid, uint steps);
	};

	uint partner;
	std::vector<item> items;
	bool entry;

	void destroy();
	void update();
	void manage();
	void unmanage();

	Box range();
};
