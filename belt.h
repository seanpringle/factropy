#ifndef _H_belt
#define _H_belt

struct Belt;

#include "sparse.h"
#include "entity.h"
#include "belt-segment.h"
#include <set>

struct Belt {
	static void reset();
	static void tick();

	static inline SparseArray<Belt> all = (MaxEntity);
	static Belt& create(uint id);
	static Belt& get(uint id);

	uint id;
	BeltSegment *segment;
	uint offset;

	void destroy();
	Belt& manage();
	Belt& unmanage();
	bool insert(uint iid);
	bool remove(uint iid);
	uint removeAny();
	uint itemAt();
};

#endif