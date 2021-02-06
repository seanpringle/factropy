
#pragma once

struct Conveyor;

#include "entity.h"
#include <list>

struct Conveyor {
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline std::map<uint,Conveyor> all;
	static Conveyor& create(uint id);
	static Conveyor& get(uint id);

	uint id;
	uint iid;
	uint offset;
	uint steps;
	uint prev;
	uint next;
	uint64_t ticked;

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
