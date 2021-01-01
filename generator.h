#ifndef _H_generator
#define _H_generator

struct Generator;

#include "sparse.h"
#include "store.h"

struct Generator {
	static void reset();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline SparseArray<Generator> all = (MaxEntity);
	static Generator& create(uint id, uint sid);
	static Generator& get(uint id);

	uint id;
	bool supplying;
	Energy monitor;

	void destroy();
	Energy consume(Energy e);
};

#endif