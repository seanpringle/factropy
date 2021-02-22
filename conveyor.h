
#pragma once

struct Conveyor;

#include "entity.h"
#include "slabmap.h"

// @todo improvements:
// a) materialize conveyor belt lines to vectors for better interation locality
// b) store relative offsets so only one gap per belt is decremented without iteration

struct Conveyor {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Conveyor,uint,&Conveyor::id> all;
	static Conveyor& create(uint id);
	static Conveyor& get(uint id);

	static inline bool rebuild = true;
	static inline std::vector<uint> leadersStraight;
	static inline std::vector<uint> leadersCircular;

	uint iid;
	uint offset;
	uint steps;
	uint prev;
	uint next;
	Conveyor* cnext;
	Conveyor* cprev;
	bool marked;
	bool managed;

	void destroy();
	Conveyor& manage();
	Conveyor& unmanage();

	void update();
	bool deliver(uint iid);

	bool insert(uint iid);
	bool remove(uint iid);
	uint removeAny();
	uint itemAt();

	Point input();
	Point output();
};
