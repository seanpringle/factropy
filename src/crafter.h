#pragma once

// Crafter components take input materials, do some work and output different
// materials. Almost everything that assembles or mines or pumps or teleports
// is at heart a Crafter. Recipes it what to do. Specs tell it how to behave.

struct Crafter;

#include "slabmap.h"
#include "item.h"
#include "fluid.h"
#include "entity.h"
#include "recipe.h"
#include <map>
#include <list>
#include <vector>

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
	Recipe *recipe, *changeRecipe;
	Energy energyUsed;
	uint completed;
	bool once;

	void destroy();
	void update();

	void craft(Recipe* recipe);
	bool craftable(Recipe* recipe);
	bool autoCraft(Item* item);
	void retool(Recipe* recipe);

	Point output();
	float inputsProgress();
	std::vector<Point> pipeConnections();
	std::vector<Point> pipeInputConnections();
	std::vector<Point> pipeOutputConnections();

	bool exporting();
	std::vector<Stack> exportItems;
	std::vector<Amount> exportFluids;

	std::vector<uint> inputPipes;
	std::vector<uint> outputPipes;
	void updatePipes();

	bool inputItemsReady();
	bool inputFluidsReady();
	bool inputMiningReady();
	bool inputCurrencyReady();

	bool outputItemsReady();
};
