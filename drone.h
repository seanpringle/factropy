#ifndef _H_drone
#define _H_drone

struct Drone;

#include "sparse.h"
#include "item.h"

struct Drone {
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline SparseArray<Drone> all = (MaxEntity);
	static Drone& create(uint id);
	static Drone& get(uint id);

	enum Stage {
		ToSrc = 1,
		ToDst,
		ToDep,
		Stranded,
	};

	uint id;
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

#endif