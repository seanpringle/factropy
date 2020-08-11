#ifndef _H_ghost
#define _H_ghost

struct Ghost;

#include "sparse.h"
#include "store.h"
#include <map>

struct Ghost {
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline std::map<uint,Ghost> all;
	static Ghost& create(uint id, uint sid);
	static Ghost& get(uint id);

	uint id;
	Store store;

	void destroy();
	void update();
	bool isConstruction();
	bool isDeconstruction();
};

#endif