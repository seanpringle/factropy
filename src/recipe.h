#pragma once

// Recipes tell Crafters how to make something, which resources to consume, how much energy
// to consume and how fast to do it.

struct Recipe;

#include "raylib-ex.h"
#include "item.h"
#include "spec.h"
#include "currency.h"
#include <map>
#include <set>
#include <vector>

struct Recipe {
	static void reset();
	static void save(const char *path);
	static void load(const char *path);
	static void saveAll(const char *path);
	static void loadAll(const char *path);

	static inline uint sequence;
	static uint next();

	static inline std::map<std::string,Recipe*> names;
	static inline std::map<uint,Recipe*> ids;
	static Recipe* byName(std::string name);
	static Recipe* get(uint id);

	uint id;
	bool licensed;
	std::string name;
	Image image;
	RenderTexture texture;
	std::vector<Part*> parts;

	std::set<std::string> tags;

	std::map<uint,uint> inputItems;
	std::map<uint,uint> outputItems;

	std::map<uint,uint> inputFluids;
	std::map<uint,uint> outputFluids;

	uint mine;
	Currency outputCurrency;
	Currency inputCurrency;

	Energy energyUsage;

	Recipe();
	Recipe(uint id, std::string name);
	~Recipe();

	static inline float miningRate = 1.0f;

	float rate(Spec* spec);
};
