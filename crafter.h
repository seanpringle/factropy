#ifndef _H_crafter
#define _H_crafter

struct Crafter;

#include "sparse.h"
#include "item.h"
#include "entity.h"
#include "recipe.h"
#include <map>

struct Crafter {
	static void reset();
	static void tick();

	static inline std::map<uint,Crafter> all;
	static Crafter& create(uint id);
	static Crafter& get(uint id);

	uint id;
	bool working;
	float progress;
	Recipe *recipe;

	void destroy();
	void update();
};

#endif
