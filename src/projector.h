#pragma once

// Projector components display an item as a bobbing rotating hologram, like Quake3 ammo.
// They connect to an adjacent Crafter or Pipe to identify the item to display.

// These may go away in favour of something better -- just a quick experiment.

struct Projector;

#include "slabmap.h"
#include "item.h"
#include "fluid.h"
#include "entity.h"
#include "recipe.h"
#include <map>

struct Projector {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Projector,&Projector::id> all;
	static Projector& create(uint id);
	static Projector& get(uint id);

	uint iid;
	uint fid;

	void destroy();
	void update();
};
