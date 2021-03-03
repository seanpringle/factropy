#pragma once

struct Computer;

#include "slabmap.h"

struct Computer {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Computer,&Computer::id> all;
	static Computer& create(uint id);
	static Computer& get(uint id);

	void destroy();
	void update();
};
