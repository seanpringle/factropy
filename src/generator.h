#pragma once

// Generator components convert fluids into energy. They link to adjacent Pipe
// networks for fuel.

struct Generator;

#include "store.h"

struct Generator {
	uint id;
	static void reset();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Generator,&Generator::id> all;
	static Generator& create(uint id, uint sid);
	static Generator& get(uint id);

	bool supplying;
	Energy energy;

	void destroy();
	Energy consume(Energy e);
};
