#ifndef _H_belt
#define _H_belt

struct Belt;

enum BeltSpot {
	BeltAny = 0,
	BeltFront,
	BeltMiddle,
	BeltBack,
};

#include "sparse.h"
#include "entity.h"
#include "belt-segment.h"
#include <set>

struct Belt {
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline SparseArray<Belt> all = (MaxEntity);
	static Belt& create(uint id);
	static Belt& get(uint id);

	uint id;
	BeltSegment *segment;
	uint offset;

	void destroy();
	Belt& manage();
	Belt& unmanage();
	bool insert(uint iid, enum BeltSpot spot);
	bool remove(uint iid, enum BeltSpot spot);
	uint removeAny(enum BeltSpot spot);
	uint itemAt(enum BeltSpot spot);
};

#endif