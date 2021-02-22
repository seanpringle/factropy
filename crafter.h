#ifndef _H_crafter
#define _H_crafter

struct Crafter;

#include "slabmap.h"
#include "item.h"
#include "fluid.h"
#include "entity.h"
#include "recipe.h"
#include <map>

struct Crafter {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Crafter,&Crafter::id> all;
	static Crafter& create(uint id);
	static Crafter& get(uint id);

	bool working;
	float progress;
	float efficiency;
	Recipe *recipe, *nextRecipe;
	Energy energyUsed;
	uint completed;

	void destroy();
	void update();
	Point output();
	float inputsProgress();
	std::vector<Point> pipeConnections();
	std::vector<Point> pipeInputConnections();
	std::vector<Point> pipeOutputConnections();

};

#endif
