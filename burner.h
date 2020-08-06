#ifndef _H_burner
#define _H_burner

struct Burner;

#include "sparse.h"
#include "store.h"

struct Burner {
	static void reset();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline SparseArray<Burner> all = (MaxEntity);
	static Burner& create(uint id);
	static Burner& get(uint id);

	uint id;
	Energy energy;
	Energy buffer;
	Store store;

	void destroy();
	Energy consume(Energy e);
};

#endif