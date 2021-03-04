#pragma once

// Loader components link up with Conveyors to load/unload Stores. They
// are essentially a subset of Arm functionality and so correspondingly
// less flexible but also faster and more efficient.

struct Loader;

#include "entity.h"
#include "conveyor.h"
#include "slabmap.h"

struct Loader {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Loader,&Loader::id> all;
	static Loader& create(uint id);
	static Loader& get(uint id);

	uint storeId;
	uint64_t pause;
	miniset<uint> filters;
	bool loading;

	void destroy();
	void update();

	Point point();

	Stack transferBeltToStore(Store& dst, Stack stack);
	Stack transferStoreToBelt(Store& src);
};
