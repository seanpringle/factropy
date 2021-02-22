#ifndef _H_burner
#define _H_burner

struct Burner;

#include "slabmap.h"
#include "store.h"

struct Burner {
	uint id;
	static void reset();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Burner,&Burner::id> all;
	static Burner& create(uint id, uint sid);
	static Burner& get(uint id);

	Energy energy;
	Energy buffer;
	Store store;

	void destroy();
	Energy consume(Energy e);
};

#endif