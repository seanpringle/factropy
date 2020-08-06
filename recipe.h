#ifndef _H_recipe
#define _H_recipe

struct Recipe;

#include "raylib.h"
#include "item.h"
#include <map>
#include <set>
#include <vector>

struct Recipe {
	static void reset();

	static inline uint sequence;
	static uint next();

	static inline std::map<std::string,Recipe*> names;
	static inline std::map<uint,Recipe*> ids;
	static Recipe* byName(std::string name);
	static Recipe* get(uint id);

	uint id;
	std::string name;
	Image image;
	RenderTexture texture;
	std::vector<Part*> parts;

	std::set<std::string> tags;

	bool mining;

	std::map<uint,uint> inputItems;
	std::map<uint,uint> outputItems;

	Energy energyUsage;

	Recipe(uint id, std::string name);
	~Recipe();
};

#endif
