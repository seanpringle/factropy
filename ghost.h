#ifndef _H_ghost
#define _H_ghost

struct Ghost;

#include "slabmap.h"
#include "store.h"

struct Ghost {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Ghost,&Ghost::id> all;
	static Ghost& create(uint id, uint sid);
	static Ghost& get(uint id);

	Store store;

	void destroy();
	void update();
	bool isConstruction();
	bool isDeconstruction();
};

#endif