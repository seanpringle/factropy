#ifndef _H_burner
#define _H_burner

struct Burner;

#include "store.h"

struct Burner {
	static void reset();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline std::map<uint,Burner> all;
	static Burner& create(uint id, uint sid);
	static Burner& get(uint id);

	uint id;
	Energy energy;
	Energy buffer;
	Store store;

	void destroy();
	Energy consume(Energy e);
};

#endif