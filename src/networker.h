#pragma once

struct Networker;

#include "slabmap.h"

struct Networker {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Networker,&Networker::id> all;
	static Networker& create(uint id);
	static Networker& get(uint id);


	void destroy();
	void update();
};
