#pragma once

struct Computer;

#include "entity.h"
#include <map>

struct Computer {
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline std::map<uint,Computer> all;
	static Computer& create(uint id);
	static Computer& get(uint id);

	uint id;

	void destroy();
	void update();
};
