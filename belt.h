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

	static inline uint Any = 0;
	static inline uint Front = 1;
	static inline uint Middle = 2;
	static inline uint Back = 3;

	uint id;
	BeltSegment *segment;
	uint offset;

	void destroy();
	Belt& manage();
	Belt& unmanage();
	bool insert(uint iid, uint area);
	bool remove(uint iid, uint area);
	uint removeAny(uint area);
	uint itemAt(uint area);
};

#endif