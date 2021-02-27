#pragma once

// Tech(nologies) are purchasable base and entity upgrades.

struct Tech;

#include "raylib.h"
#include "item.h"
#include "spec.h"
#include "currency.h"
#include <map>
#include <set>
#include <vector>

struct Tech {
	static void reset();
	static void saveAll(const char *path);
	static void loadAll(const char *path);

	static inline uint sequence;
	static uint next();

	static inline std::map<std::string,Tech*> names;
	static inline std::map<uint,Tech*> ids;
	static Tech* byName(std::string name);
	static Tech* get(uint id);

	uint id;
	std::string name;
	Image image;
	RenderTexture texture;
	std::vector<Part*> parts;

	std::set<std::string> tags;

	bool bought;
	Currency cost;

	float miningRate;

	Tech(uint id, std::string name);
	~Tech();

	void buy();
};
