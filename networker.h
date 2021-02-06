#pragma once

struct Networker;

#include "entity.h"
#include <map>

struct Networker {
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline std::map<uint,Networker> all;
	static Networker& create(uint id);
	static Networker& get(uint id);

	uint id;

	void destroy();
	void update();
};
