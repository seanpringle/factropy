#ifndef _H_belt
#define _H_belt

struct Belt;

#include "sparse.h"
#include "entity.h"
#include <set>

struct BeltSegment {
	static inline std::set<BeltSegment*> all;

	std::set<Belt*> belts;
	BeltSegment();
	~BeltSegment();
};

struct Belt {
	static void reset();
	static void tick();

	static inline SparseArray<Belt> all = (MaxEntity);
	static Belt& create(uint id);
	static Belt& get(uint id);

	uint id;
	BeltSegment *segment;

	void destroy();
	Belt& manage();
	Belt& unmanage();
};

#endif