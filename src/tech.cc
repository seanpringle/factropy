#include "common.h"
#include "ledger.h"
#include "recipe.h"
#include "tech.h"

void Tech::reset() {
	for (auto pair: ids) {
		delete pair.second;
	}
	names.clear();
	ids.clear();
}

uint Tech::next() {
	return sequence++;
}

Tech::Tech(uint id, std::string name) {
	ensuref(names.count(name) == 0, "duplicate tech %s", name.c_str());
	this->name = name;
	this->id = id;
	names[name] = this;
	ids[id] = this;

	bought = false;
	cost = 0;

	miningRate = 0.0f;

	ZERO(image);
	ZERO(texture);
}

Tech::~Tech() {
	UnloadImage(image);
}

Tech* Tech::byName(std::string name) {
	ensuref(names.count(name) == 1, "unknown tech %s", name.c_str());
	return names[name];
}

Tech* Tech::get(uint id) {
	ensuref(ids.count(id) == 1, "unknown tech %d", id);
	return ids[id];
}

void Tech::buy() {
	ensure(!bought);

	bought = true;
	Ledger::transact(-cost, name);

	if (miningRate > 0.0f) {
		Recipe::miningRate = std::max(Recipe::miningRate, miningRate);
	}

	for (auto spec: licenseSpecs) {
		spec->licensed = true;
	}

	for (auto recipe: licenseRecipes) {
		recipe->licensed = true;
	}
}