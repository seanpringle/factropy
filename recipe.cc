#include "common.h"
#include "recipe.h"

void Recipe::reset() {
	for (auto pair: ids) {
		delete pair.second;
	}
	names.clear();
	ids.clear();
}

uint Recipe::next() {
	return sequence++;
}

Recipe::Recipe(uint id, std::string name) {
	ensuref(names.count(name) == 0, "duplicate recipe %s", name.c_str());
	this->name = name;
	this->id = id;
	names[name] = this;
	ids[id] = this;

	mining = false;
	energyUsage = 0;
	ZERO(image);
	ZERO(texture);
}

Recipe::~Recipe() {
	UnloadImage(image);
}

Recipe* Recipe::byName(std::string name) {
	ensuref(names.count(name) == 1, "unknown recipe %s", name.c_str());
	return names[name];
}

Recipe* Recipe::get(uint id) {
	ensuref(ids.count(id) == 1, "unknown recipe %d", id);
	return ids[id];
}

